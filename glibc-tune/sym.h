
struct lsym {
	char *name;
	unsigned long addr;
};

/* All symbols must be in the same DSO specified by fn/baseaddr */
int find_symbol(char *fn, struct lsym *lsym, unsigned long baseaddr);
