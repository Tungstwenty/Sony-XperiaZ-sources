// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.content.browser;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.MediaController;
import android.widget.ProgressBar;
import android.widget.TextView;

import org.chromium.base.CalledByNative;
import org.chromium.base.JNINamespace;
import org.chromium.base.ThreadUtils;
import org.chromium.ui.base.ViewAndroid;
import org.chromium.ui.base.ViewAndroidDelegate;
import org.chromium.ui.base.WindowAndroid;

@JNINamespace("content")
public class ContentVideoView extends FrameLayout implements MediaController.MediaPlayerControl,
        SurfaceHolder.Callback, View.OnTouchListener, View.OnKeyListener, ViewAndroidDelegate  {


    private static final String TAG = "ContentVideoView";

    /* Do not change these values without updating their counterparts
     * in include/media/mediaplayer.h!
     */
    private static final int MEDIA_NOP = 0; // interface test message
    private static final int MEDIA_PREPARED = 1;
    private static final int MEDIA_PLAYBACK_COMPLETE = 2;
    private static final int MEDIA_BUFFERING_UPDATE = 3;
    private static final int MEDIA_SEEK_COMPLETE = 4;
    private static final int MEDIA_SET_VIDEO_SIZE = 5;
    private static final int MEDIA_ERROR = 100;
    private static final int MEDIA_INFO = 200;

    /**
     * Keep these error codes in sync with the code we defined in
     * MediaPlayerListener.java.
     */
    public static final int MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 2;
    public static final int MEDIA_ERROR_INVALID_CODE = 3;

    // all possible internal states
    private static final int STATE_ERROR              = -1;
    private static final int STATE_IDLE               = 0;
    private static final int STATE_PLAYING            = 1;
    private static final int STATE_PAUSED             = 2;
    private static final int STATE_PLAYBACK_COMPLETED = 3;

    private SurfaceHolder mSurfaceHolder;
    private int mVideoWidth;
    private int mVideoHeight;
    private int mCurrentBufferPercentage;
    private int mDuration;
    private MediaController mMediaController;
    private boolean mCanPause;
    private boolean mCanSeekBack;
    private boolean mCanSeekForward;

    // Native pointer to C++ ContentVideoView object.
    private long mNativeContentVideoView;

    // webkit should have prepared the media
    private int mCurrentState = STATE_IDLE;

    // Strings for displaying media player errors
    private String mPlaybackErrorText;
    private String mUnknownErrorText;
    private String mErrorButton;
    private String mErrorTitle;
    private String mVideoLoadingText;

    // This view will contain the video.
    private VideoSurfaceView mVideoSurfaceView;

    // Progress view when the video is loading.
    private View mProgressView;

    // The ViewAndroid is used to keep screen on during video playback.
    private ViewAndroid mViewAndroid;

    private final ContentVideoViewClient mClient;

    private class VideoSurfaceView extends SurfaceView {

        public VideoSurfaceView(Context context) {
            super(context);
        }

        @Override
        protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
            int width = getDefaultSize(mVideoWidth, widthMeasureSpec);
            int height = getDefaultSize(mVideoHeight, heightMeasureSpec);
            if (mVideoWidth > 0 && mVideoHeight > 0) {
                if (mVideoWidth * height  > width * mVideoHeight) {
                    height = width * mVideoHeight / mVideoWidth;
                } else if (mVideoWidth * height  < width * mVideoHeight) {
                    width = height * mVideoWidth / mVideoHeight;
                }
            }
            setMeasuredDimension(width, height);
        }
    }

    private static class ProgressView extends LinearLayout {

        private final ProgressBar mProgressBar;
        private final TextView mTextView;

        public ProgressView(Context context, String videoLoadingText) {
            super(context);
            setOrientation(LinearLayout.VERTICAL);
            setLayoutParams(new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT));
            mProgressBar = new ProgressBar(context, null, android.R.attr.progressBarStyleLarge);
            mTextView = new TextView(context);
            mTextView.setText(videoLoadingText);
            addView(mProgressBar);
            addView(mTextView);
        }
    }

    private static class FullScreenMediaController extends MediaController {

        View mVideoView;

        public FullScreenMediaController(Context context, View video) {
            super(context);
            mVideoView = video;
        }

        @Override
        public void show() {
            super.show();
            if (mVideoView != null) {
                mVideoView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
            }
        }

        @Override
        public void hide() {
            if (mVideoView != null) {
                mVideoView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
            }
            super.hide();
        }
    }

    private final Runnable mExitFullscreenRunnable = new Runnable() {
        @Override
        public void run() {
            exitFullscreen(true);
        }
    };

    private ContentVideoView(Context context, long nativeContentVideoView,
            ContentVideoViewClient client) {
        super(context);
        mNativeContentVideoView = nativeContentVideoView;
        mViewAndroid = new ViewAndroid(new WindowAndroid(context.getApplicationContext()), this);
        mClient = client;
        initResources(context);
        mCurrentBufferPercentage = 0;
        mVideoSurfaceView = new VideoSurfaceView(context);
        setBackgroundColor(Color.BLACK);
        showContentVideoView();
        setVisibility(View.VISIBLE);
        mClient.onShowCustomView(this);
    }

    private void initResources(Context context) {
        if (mPlaybackErrorText != null) return;
        mPlaybackErrorText = context.getString(
                org.chromium.content.R.string.media_player_error_text_invalid_progressive_playback);
        mUnknownErrorText = context.getString(
                org.chromium.content.R.string.media_player_error_text_unknown);
        mErrorButton = context.getString(
                org.chromium.content.R.string.media_player_error_button);
        mErrorTitle = context.getString(
                org.chromium.content.R.string.media_player_error_title);
        mVideoLoadingText = context.getString(
                org.chromium.content.R.string.media_player_loading_video);
    }

    private void showContentVideoView() {
        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);
        this.addView(mVideoSurfaceView, layoutParams);
        View progressView = mClient.getVideoLoadingProgressView();
        if (progressView != null) {
            mProgressView = progressView;
        } else {
            mProgressView = new ProgressView(getContext(), mVideoLoadingText);
        }
        this.addView(mProgressView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.CENTER));
        mVideoSurfaceView.setZOrderOnTop(true);
        mVideoSurfaceView.setOnKeyListener(this);
        mVideoSurfaceView.setOnTouchListener(this);
        mVideoSurfaceView.getHolder().addCallback(this);
        mVideoSurfaceView.setFocusable(true);
        mVideoSurfaceView.setFocusableInTouchMode(true);
        mVideoSurfaceView.requestFocus();
    }

    @CalledByNative
    public void onMediaPlayerError(int errorType) {
        Log.d(TAG, "OnMediaPlayerError: " + errorType);
        if (mCurrentState == STATE_ERROR || mCurrentState == STATE_PLAYBACK_COMPLETED) {
            return;
        }

        // Ignore some invalid error codes.
        if (errorType == MEDIA_ERROR_INVALID_CODE) {
            return;
        }

        mCurrentState = STATE_ERROR;
        if (mMediaController != null) {
            mMediaController.hide();
        }

        /* Pop up an error dialog so the user knows that
         * something bad has happened. Only try and pop up the dialog
         * if we're attached to a window. When we're going away and no
         * longer have a window, don't bother showing the user an error.
         *
         * TODO(qinmin): We need to review whether this Dialog is OK with
         * the rest of the browser UI elements.
         */
        if (getWindowToken() != null) {
            String message;

            if (errorType == MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
                message = mPlaybackErrorText;
            } else {
                message = mUnknownErrorText;
            }

            try {
                new AlertDialog.Builder(getContext())
                    .setTitle(mErrorTitle)
                    .setMessage(message)
                    .setPositiveButton(mErrorButton,
                            new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int whichButton) {
                            /* Inform that the video is over.
                             */
                            onCompletion();
                        }
                    })
                    .setCancelable(false)
                    .show();
            } catch (RuntimeException e) {
                Log.e(TAG, "Cannot show the alert dialog, error message: " + message, e);
            }
        }
    }

    @CalledByNative
    private void onVideoSizeChanged(int width, int height) {
        mVideoWidth = width;
        mVideoHeight = height;
        // This will trigger the SurfaceView.onMeasure() call.
        mVideoSurfaceView.getHolder().setFixedSize(mVideoWidth, mVideoHeight);
    }

    @CalledByNative
    private void onBufferingUpdate(int percent) {
        mCurrentBufferPercentage = percent;
    }

    @CalledByNative
    private void onPlaybackComplete() {
        onCompletion();
    }

    @CalledByNative
    private void onUpdateMediaMetadata(
            int videoWidth,
            int videoHeight,
            int duration,
            boolean canPause,
            boolean canSeekBack,
            boolean canSeekForward) {
        mProgressView.setVisibility(View.GONE);
        mDuration = duration;
        mCanPause = canPause;
        mCanSeekBack = canSeekBack;
        mCanSeekForward = canSeekForward;
        mCurrentState = isPlaying() ? STATE_PLAYING : STATE_PAUSED;
        if (mMediaController != null) {
            mMediaController.setEnabled(true);
            // If paused , should show the controller for ever.
            if (isPlaying())
                mMediaController.show();
            else
                mMediaController.show(0);
        }

        onVideoSizeChanged(videoWidth, videoHeight);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        mVideoSurfaceView.setFocusable(true);
        mVideoSurfaceView.setFocusableInTouchMode(true);
        if (isInPlaybackState() && mMediaController != null) {
            mMediaController.show();
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mSurfaceHolder = holder;
        openVideo();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mNativeContentVideoView != 0) {
            nativeSetSurface(mNativeContentVideoView, null);
        }
        mSurfaceHolder = null;
        post(mExitFullscreenRunnable);
    }

    private void setMediaController(MediaController controller) {
        if (mMediaController != null) {
            mMediaController.hide();
        }
        mMediaController = controller;
        attachMediaController();
    }

    private void attachMediaController() {
        if (mMediaController != null) {
            mMediaController.setMediaPlayer(this);
            mMediaController.setAnchorView(mVideoSurfaceView);
            mMediaController.setEnabled(false);
        }
    }

    @CalledByNative
    private void openVideo() {
        if (mSurfaceHolder != null) {
            mCurrentState = STATE_IDLE;
            mCurrentBufferPercentage = 0;
            setMediaController(new FullScreenMediaController(getContext(), this));
            if (mNativeContentVideoView != 0) {
                nativeUpdateMediaMetadata(mNativeContentVideoView);
                nativeSetSurface(mNativeContentVideoView,
                        mSurfaceHolder.getSurface());
            }
        }
    }

    private void onCompletion() {
        mCurrentState = STATE_PLAYBACK_COMPLETED;
        if (mMediaController != null) {
            mMediaController.hide();
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (isInPlaybackState() && mMediaController != null &&
                event.getAction() == MotionEvent.ACTION_DOWN) {
            toggleMediaControlsVisiblity();
        }
        return true;
    }

    @Override
    public boolean onTrackballEvent(MotionEvent ev) {
        if (isInPlaybackState() && mMediaController != null) {
            toggleMediaControlsVisiblity();
        }
        return false;
    }

    @Override
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        boolean isKeyCodeSupported = keyCode != KeyEvent.KEYCODE_BACK &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_UP &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_DOWN &&
                                     keyCode != KeyEvent.KEYCODE_VOLUME_MUTE &&
                                     keyCode != KeyEvent.KEYCODE_CALL &&
                                     keyCode != KeyEvent.KEYCODE_MENU &&
                                     keyCode != KeyEvent.KEYCODE_SEARCH &&
                                     keyCode != KeyEvent.KEYCODE_ENDCALL;
        if (isInPlaybackState() && isKeyCodeSupported && mMediaController != null) {
            if (keyCode == KeyEvent.KEYCODE_HEADSETHOOK ||
                    keyCode == KeyEvent.KEYCODE_MEDIA_PLAY_PAUSE) {
                if (isPlaying()) {
                    pause();
                    mMediaController.show();
                } else {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_PLAY) {
                if (!isPlaying()) {
                    start();
                    mMediaController.hide();
                }
                return true;
            } else if (keyCode == KeyEvent.KEYCODE_MEDIA_STOP
                    || keyCode == KeyEvent.KEYCODE_MEDIA_PAUSE) {
                if (isPlaying()) {
                    pause();
                    mMediaController.show();
                }
                return true;
            } else {
                toggleMediaControlsVisiblity();
            }
        } else if (keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_UP) {
            exitFullscreen(false);
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_MENU || keyCode == KeyEvent.KEYCODE_SEARCH) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private void toggleMediaControlsVisiblity() {
        if (mMediaController.isShowing()) {
            mMediaController.hide();
        } else {
            mMediaController.show();
        }
    }

    private boolean isInPlaybackState() {
        return (mCurrentState != STATE_ERROR && mCurrentState != STATE_IDLE);
    }

    @Override
    public void start() {
        if (isInPlaybackState()) {
            if (mNativeContentVideoView != 0) {
                nativePlay(mNativeContentVideoView);
            }
            mCurrentState = STATE_PLAYING;
        }
    }

    @Override
    public void pause() {
        if (isInPlaybackState()) {
            if (isPlaying()) {
                if (mNativeContentVideoView != 0) {
                    nativePause(mNativeContentVideoView);
                }
                mCurrentState = STATE_PAUSED;
            }
        }
    }

    // cache duration as mDuration for faster access
    @Override
    public int getDuration() {
        if (isInPlaybackState()) {
            if (mDuration > 0) {
                return mDuration;
            }
            if (mNativeContentVideoView != 0) {
                mDuration = nativeGetDurationInMilliSeconds(mNativeContentVideoView);
            } else {
                mDuration = 0;
            }
            return mDuration;
        }
        mDuration = -1;
        return mDuration;
    }

    @Override
    public int getCurrentPosition() {
        if (isInPlaybackState() && mNativeContentVideoView != 0) {
            return nativeGetCurrentPosition(mNativeContentVideoView);
        }
        return 0;
    }

    @Override
    public void seekTo(int msec) {
        if (mNativeContentVideoView != 0) {
            nativeSeekTo(mNativeContentVideoView, msec);
        }
    }

    @Override
    public boolean isPlaying() {
        return mNativeContentVideoView != 0 && nativeIsPlaying(mNativeContentVideoView);
    }

    @Override
    public int getBufferPercentage() {
        return mCurrentBufferPercentage;
    }

    @Override
    public boolean canPause() {
        return mCanPause;
    }

    @Override
    public boolean canSeekBackward() {
        return mCanSeekBack;
    }

    @Override
    public boolean canSeekForward() {
        return mCanSeekForward;
    }

    @Override
    public int getAudioSessionId() {
        return 0;
    }

    @CalledByNative
    private static ContentVideoView createContentVideoView(
            Context context, long nativeContentVideoView, ContentVideoViewClient client) {
        ThreadUtils.assertOnUiThread();
        // The context needs be Activity to create the ContentVideoView correctly.
        if (!(context instanceof Activity)) {
            Log.w(TAG, "Wrong type of context, can't create fullscreen video");
            return null;
        }
        return new ContentVideoView(context, nativeContentVideoView, client);
    }

    private void removeMediaController() {
        if (mMediaController != null) {
            mMediaController.setEnabled(false);
            mMediaController.hide();
            mMediaController = null;
        }
    }

    public void removeSurfaceView() {
        removeView(mVideoSurfaceView);
        removeView(mProgressView);
        mVideoSurfaceView = null;
        mProgressView = null;
    }

    public void exitFullscreen(boolean relaseMediaPlayer) {
        destroyContentVideoView(false);
        if (mNativeContentVideoView != 0) {
            nativeExitFullscreen(mNativeContentVideoView, relaseMediaPlayer);
            mNativeContentVideoView = 0;
        }
    }

    /**
     * This method shall only be called by native and exitFullscreen,
     * To exit fullscreen, use exitFullscreen in Java.
     */
    @CalledByNative
    private void destroyContentVideoView(boolean nativeViewDestroyed) {
        if (mVideoSurfaceView != null) {
            removeMediaController();
            removeSurfaceView();
            setVisibility(View.GONE);

            // To prevent re-entrance, call this after removeSurfaceView.
            mClient.onDestroyContentVideoView();
        }
        if (nativeViewDestroyed) {
            mNativeContentVideoView = 0;
        }
    }

    public static ContentVideoView getContentVideoView() {
        return nativeGetSingletonJavaContentVideoView();
    }

    @Override
    public boolean onTouchEvent(MotionEvent ev) {
        return true;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_UP) {
            exitFullscreen(false);
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public View acquireAnchorView() {
        View anchorView = new View(getContext());
        addView(anchorView);
        return anchorView;
    }

    @Override
    public void setAnchorViewPosition(View view, float x, float y, float width, float height) {
        Log.e(TAG, "setAnchorViewPosition isn't implemented");
    }

    @Override
    public void releaseAnchorView(View anchorView) {
        removeView(anchorView);
    }

    @CalledByNative
    private long getNativeViewAndroid() {
        return mViewAndroid.getNativePointer();
    }

    private static native ContentVideoView nativeGetSingletonJavaContentVideoView();
    private native void nativeExitFullscreen(long nativeContentVideoView,
            boolean relaseMediaPlayer);
    private native int nativeGetCurrentPosition(long nativeContentVideoView);
    private native int nativeGetDurationInMilliSeconds(long nativeContentVideoView);
    private native void nativeUpdateMediaMetadata(long nativeContentVideoView);
    private native int nativeGetVideoWidth(long nativeContentVideoView);
    private native int nativeGetVideoHeight(long nativeContentVideoView);
    private native boolean nativeIsPlaying(long nativeContentVideoView);
    private native void nativePause(long nativeContentVideoView);
    private native void nativePlay(long nativeContentVideoView);
    private native void nativeSeekTo(long nativeContentVideoView, int msec);
    private native void nativeSetSurface(long nativeContentVideoView, Surface surface);
}