// Secuencia Cruz en FLASH (sin cambios)
static const Patron cruz_seq[] PROGMEM = {
	{(1 << PD7), 5},
	{(1 << PD4), 5},
	{(1 << PD2), 1},
	{(1 << PD4) | (1 << PD7), 5},
	{(1 << PD3), 1},
	{(1 << PD5), 5},
	{(1 << PD2), 1},
	{(1 << PD4) | (1 << PD6), 5},
	{(1 << PD3), 1}
};


static const Patron triangulo_seq[] PROGMEM = {
	{(1 << PD7), 5},
	{(1 << PD7), 5},
	{(1 << PD4), 2},
	{(1 << PD2), 2},
	{(1 << PD4), 5},
	{(1 << PD7), 5},
	{(1 << PD6) | (1 << PD5), 5},
	{(1 << PD3), 1},
	{(1 << PD6), 20},
	{(1 << PD6), 10},
	{(1 << PD5), 2}
};


static const Patron circulo_seq[] PROGMEM = {
	/* BAJA SELENOIDE */
	{(1 << PD2), 5},
	/* C?RCULO */
	{ (1 << PD7),  1 },
	{ (1 << PD4),  4 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD7),  4 },
	{ (1 << PD4),  1 },
	{ (1 << PD7), 12 },


	{ (1 << PD5),  1 },
	{ (1 << PD7),  4 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD7),  1 },
	{ (1 << PD5),  4 },
	{ (1 << PD7),  1 },
	{ (1 << PD5), 12 },



	{ (1 << PD6),  1 },
	{ (1 << PD5),  4 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD5),  1 },
	{ (1 << PD6),  4 },
	{ (1 << PD5),  1 },
	{ (1 << PD6), 12 },



	{ (1 << PD4),  1 },
	{ (1 << PD6),  4 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  2 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  1 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  2 },
	{ (1 << PD6),  1 },
	{ (1 << PD4),  4 },
	{ (1 << PD6),  1 },
	{ (1 << PD4), 15 },
	
	/* SUBE SELENOIDE */
	{(1 << PD3), 1},
};

static const Patron Rana_seq[] PROGMEM = {
	{(1 << PD2), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD3), 1},
	
	{(1 << PD7), 39},
	{(1 << PD5), 20},
	{(1 << PD2), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 13},
	{(1 << PD4), 1},
	{(1 << PD6), 11},
	{(1 << PD3), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD2), 1},
	{(1 << PD7), 9},
	{(1 << PD5), 1},
	{(1 << PD7), 10},
	{(1 << PD5), 1},
	{(1 << PD7), 7},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 2},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 1},
	{(1 << PD7), 2},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 2},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 4},
	{(1 << PD4), 1},
	{(1 << PD7), 3},
	{(1 << PD4), 7},
	{(1 << PD7), 1},
	{(1 << PD4), 8},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 3},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 1},
	{(1 << PD4), 2},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 8},
	{(1 << PD4), 5},
	{(1 << PD6), 1},
	{(1 << PD4), 6},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD5), 2},
	{(1 << PD6), 1},
	{(1 << PD5), 3},
	{(1 << PD6), 2},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 2},
	{(1 << PD7), 1},
	{(1 << PD5), 7},
	{(1 << PD7), 1},
	{(1 << PD5), 5},
	{(1 << PD7), 1},
	{(1 << PD5), 4},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD3), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD3), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 3},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD3), 1},
	{(1 << PD6), 5},
	{(1 << PD4), 18},
	{(1 << PD2), 1},
	{(1 << PD7), 1},
	{(1 << PD5), 1},
	{(1 << PD7), 4},
	{(1 << PD4), 1},
	{(1 << PD7), 10},
	{(1 << PD5), 1},
	{(1 << PD7), 5},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 1},
	{(1 << PD7), 1},
	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 5},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 1},
	{(1 << PD4), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 3},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 2},
	{(1 << PD5), 2},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 1},
	{(1 << PD5), 3},
	{(1 << PD3), 2},
	{(1 << PD5), 10},
	{(1 << PD7), 5},
	{(1 << PD2), 1},




	{(1 << PD6), 3},
	{(1 << PD4), 1},
	{(1 << PD6), 1},

	{(1 << PD4), 3},
	{(1 << PD7), 1},
	{(1 << PD4), 1},

	{(1 << PD7), 3},
	{(1 << PD5), 1},
	{(1 << PD7), 1},

	{(1 << PD5), 3},
	{(1 << PD6), 1},
	{(1 << PD5), 1},
	{(1 << PD6), 4},
	{(1 << PD3), 1},
	{(1 << PD3), 1},
};

static void Manzana(void) {
	PosicionamientoManzana();
	PORTD = (1 << PD2); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(4);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(6);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(4);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(12);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(7);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(6);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(7);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(6);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(4);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD4); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(16);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(17);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(5);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(7);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(9);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(7);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(6);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(2);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD6); AUTOGEN_DELAY(25);
	PORTD = (1 << PD5); AUTOGEN_DELAY(2);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(12);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(10);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(3);
	PORTD = (1 << PD7); AUTOGEN_DELAY(1);
	PORTD = (1 << PD5); AUTOGEN_DELAY(1);
	PORTD = (1 << PD3); AUTOGEN_DELAY(1);
}




