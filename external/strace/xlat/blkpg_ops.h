/* Generated by ./xlat/gen.sh from ./xlat/blkpg_ops.in; do not edit. */

static const struct xlat blkpg_ops[] = {
#if defined(BLKPG_ADD_PARTITION) || (defined(HAVE_DECL_BLKPG_ADD_PARTITION) && HAVE_DECL_BLKPG_ADD_PARTITION)
	XLAT(BLKPG_ADD_PARTITION),
#endif
#if defined(BLKPG_DEL_PARTITION) || (defined(HAVE_DECL_BLKPG_DEL_PARTITION) && HAVE_DECL_BLKPG_DEL_PARTITION)
	XLAT(BLKPG_DEL_PARTITION),
#endif
	XLAT_END
};
