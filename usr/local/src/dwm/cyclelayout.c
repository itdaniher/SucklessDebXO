void
cyclelayout() {
	lt[sellt]++; 
	if (lt[sellt] == layouts + LENGTH(layouts)) lt[sellt] = layouts;
	if(sel) arrange();
	else drawbar();
}
