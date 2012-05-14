void
grid() {
	unsigned int i, n, cx, cy, cw, ch, aw, ah, cols, rows;
	Client *c;

	for(n = 0, c = nexttiled(clients); c; c = nexttiled(c->next))
		n++;

	/* grid dimensions */
	for(cols = 0; cols <= n/2; cols++)
		if(cols*cols >= n)
			break;
	rows = (cols && (cols - 1) * cols >= n) ? cols - 1 : cols;

	/* window geoms (cell height/width) */
	ch = wh / (rows ? rows : 1);
	cw = ww / (cols ? cols : 1);
	for(i = 0, c = nexttiled(clients); c; c = nexttiled(c->next)) {
		cx = (i / rows) * cw;
		cy = (i % rows) * ch + bh;
		/* adjust height/width of last row/column's windows */
		ah = ((i + 1) % rows == 0) ? wh - ch * rows : 0;
		aw = (i >= rows * (cols - 1)) ? ww - cw * cols : 0;
		resize(c, cx, cy, cw - 2 * c->bw + aw, ch - 2 * c->bw + ah, False);
		i++;
	}
}

