inline char *stpcpy(char *d, char *s)
{
	do {
		*d = *(s++);
		if (!(*d)) return d;
		d++;
	} while(1);
}
