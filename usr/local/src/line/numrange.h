#ifndef NUMRANGE_H
#define NUMRANGE_H

#ifndef NR_TYPE
#define NR_TYPE int
#endif

/* values for mk_range_iter flags: */

#define NR_SCATTER	0x01	/* ranges need not be ascending & distinct */
#define NR_BIDIR	0x02	/* can have m-n with n < m */
#define NR_RELATIVE	0x04	/* m:n means m to m+n-1 */

/* structure returned by get_cur_range: */

struct numrange
	{
	NR_TYPE nr_first, nr_last;
	NR_TYPE nr_cur;
	int nr_flags;
	};

/* values for nr_flags: */
#define NR_OPNBEG	0x01	/* open beginning: -n */
#define NR_OPNEND	0x02	/* open end:      m-  */
#define NR_SINGLETON	0x04	/* "range" was single n, not m-n */

#ifdef __STDC__

extern struct range_iter *mk_range_iter(char *, int);
extern struct range_iter *add_range_iter(struct range_iter *, char *);
extern NR_TYPE do_range_iter(struct range_iter *, int *);
extern void rst_range_iter(struct range_iter *);
extern int in_range_iter(NR_TYPE, struct range_iter *);
extern int in2_range_iter(NR_TYPE, NR_TYPE, struct range_iter *);
extern const struct numrange *get_cur_range(struct range_iter *);
extern int get_num_ranges(struct range_iter *);
extern int skip_next_range(struct range_iter *);
extern int done_range_iter(struct range_iter *);

#endif

extern struct range_iter *mk_range_iter();
extern struct range_iter *add_range_iter();
extern NR_TYPE do_range_iter();
extern void rst_range_iter();
extern const struct numrange *get_cur_range();

#endif
