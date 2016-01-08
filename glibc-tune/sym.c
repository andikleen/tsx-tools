/* Search for symbols in ELF files */
#include <unistd.h>
#include <sys/fcntl.h>
#include <libelf.h>
#include <fcntl.h>
#include <stdio.h>
#include <gelf.h>
#include <stdlib.h>
#include <string.h>

#include "sym.h"

int find_symbol(char *fn, struct lsym *lsym, unsigned long baseaddr)
{
	struct lsym *ls;
	int num = 0;
	for (ls = lsym; ls->name; ls++)
		num++;

	elf_version(EV_CURRENT);
	int fd = open(fn, O_RDONLY);
	if (fd < 0)
		return -1;

	long ret = -1;
	Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
	if (elf == NULL)
		goto out;

	GElf_Ehdr header;
	if (!gelf_getehdr(elf, &header))
		goto out_elf;

	Elf_Scn *section = NULL;
	int found = 0;
	while (found < num && (section = elf_nextscn(elf, section)) != 0) {
		GElf_Shdr shdr, *sh;
		sh = gelf_getshdr(section, &shdr);

		if (sh->sh_type == SHT_SYMTAB || sh->sh_type == SHT_DYNSYM) {
			Elf_Data *data = elf_getdata(section, NULL);
			GElf_Sym *sym, symbol;
			int j;

			unsigned numsym = sh->sh_size / sh->sh_entsize;
			for (j = 0; j < numsym; j++) {
				sym = gelf_getsymshndx(data, NULL, j, &symbol, NULL);
				char *symname = elf_strptr(elf, shdr.sh_link, sym->st_name);
				for (ls = lsym; ls->name; ls++) {
					if (!strcmp(symname, ls->name)) {
						Elf_Scn *oscn = elf_getscn(elf, sym->st_shndx);
						GElf_Shdr oshdr, *osh;
						osh = gelf_getshdr(oscn, &oshdr);
						ls->addr = (sym->st_value - osh->sh_addr) + osh->sh_offset + baseaddr;
						found++;
						if (found == num)
							break;
					}
				}
			}
		}
	}
	if (found == num)
		ret = 0;

out_elf:
	elf_end(elf);

out:
	close(fd);
	return ret;
}
