#ifndef EXTERNAL_H
#define EXTERNAL_H

extern size_t __gc_stack_top, __gc_stack_bottom;

extern void *Barray(aint *args, aint bn);
extern void *Bclosure(aint *args, aint bn);
extern void *Belem(void *p, aint i);
extern void *Bsexp(aint *args, aint bn);
extern void *Bsta(void *x, aint i, void *v);
extern void *Bstring(aint *args);
extern void *Lstring(aint *args);
extern void Bmatch_failure(void *v, char *fname, aint line, aint col);

extern aint Barray_patt(void *d, aint n);
extern aint Barray_tag_patt(void *x);
extern aint Bclosure_tag_patt(void *x);
extern aint Bsexp_tag_patt(void *x);
extern aint Bstring_patt(void *x, void *y);
extern aint Bstring_tag_patt(void *x);
extern aint Btag(void *d, aint t, aint n);
extern aint Llength(void *p);
extern aint Lread();
extern aint LtagHash(char *);
extern aint Lwrite(aint n);

#endif //EXTERNAL_H