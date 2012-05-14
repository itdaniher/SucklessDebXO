void
monocle(void) {
   unsigned int n;
   Client *c;

   for(n = 0, c = nexttiled(clients); c && n < 2; c = nexttiled(c->next), n++);
   for(c = nexttiled(clients); c; c = nexttiled(c->next)) {
      adjustborder(c, n == 1 ? 0 : borderpx);
      resize(c, wx, wy, ww - 2 * c->bw, wh - 2 * c->bw, resizehints);
   }
}

