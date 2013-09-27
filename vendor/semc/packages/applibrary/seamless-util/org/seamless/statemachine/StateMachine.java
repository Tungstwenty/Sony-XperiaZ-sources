 /*
 * Copyright (C) 2012 4th Line GmbH, Switzerland
 *
 * The contents of this file are subject to the terms of either the GNU
 * Lesser General Public License Version 2 or later ("LGPL") or the
 * Common Development and Distribution License Version 1 or later
 * ("CDDL") (collectively, the "License"). You may not use this file
 * except in compliance with the License. See LICENSE.txt for more
 * information.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
package org.seamless.statemachine;

/**
 * @author Christian Bauer
 */
public interface StateMachine<S> {

    public static final String METHOD_CURRENT_STATE = "getCurrentState";
    public static final String METHOD_FORCE_STATE = "forceState";

    public S getCurrentState();
    public void forceState(Class<? extends S> state);
}
