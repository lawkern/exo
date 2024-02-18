/* /////////////////////////////////////////////////////////////////////////// */
/* (c) copyright 2024 Lawrence D. Kern /////////////////////////////////////// */
/* /////////////////////////////////////////////////////////////////////////// */

#define FONT_WIDTH 8
#define FONT_HEIGHT 10
#define FONT_LEADING 12
#define FONT_SCALE 1

#define ALIGN_TEXT_VERTICALLY(y, h) ((y) + ((h) / 2) - (FONT_SCALE * FONT_HEIGHT / 2))
#define ADVANCE_TEXT_LINE(y) ((y) + (FONT_SCALE * FONT_LEADING))

struct bitmap_glyph
{
   u8 memory[FONT_WIDTH * FONT_HEIGHT / 8];
};

struct bitmap_font
{
   bitmap_glyph glyphs[256];
};

global bitmap_font font;

function void initialize_font(void)
{
   font.glyphs['A'].memory[0] = 0b00111100; font.glyphs['B'].memory[0] = 0b01111100;
   font.glyphs['A'].memory[1] = 0b01000010; font.glyphs['B'].memory[1] = 0b01000010;
   font.glyphs['A'].memory[2] = 0b01000010; font.glyphs['B'].memory[2] = 0b01000010;
   font.glyphs['A'].memory[3] = 0b01000010; font.glyphs['B'].memory[3] = 0b01000010;
   font.glyphs['A'].memory[4] = 0b01111110; font.glyphs['B'].memory[4] = 0b01111100;
   font.glyphs['A'].memory[5] = 0b01000010; font.glyphs['B'].memory[5] = 0b01000010;
   font.glyphs['A'].memory[6] = 0b01000010; font.glyphs['B'].memory[6] = 0b01000010;
   font.glyphs['A'].memory[7] = 0b01000010; font.glyphs['B'].memory[7] = 0b01000010;
   font.glyphs['A'].memory[8] = 0b01000010; font.glyphs['B'].memory[8] = 0b01111100;
   font.glyphs['A'].memory[9] = 0b00000000; font.glyphs['B'].memory[9] = 0b00000000;

   font.glyphs['C'].memory[0] = 0b00111100; font.glyphs['D'].memory[0] = 0b01111100;
   font.glyphs['C'].memory[1] = 0b01000010; font.glyphs['D'].memory[1] = 0b01000010;
   font.glyphs['C'].memory[2] = 0b01000000; font.glyphs['D'].memory[2] = 0b01000010;
   font.glyphs['C'].memory[3] = 0b01000000; font.glyphs['D'].memory[3] = 0b01000010;
   font.glyphs['C'].memory[4] = 0b01000000; font.glyphs['D'].memory[4] = 0b01000010;
   font.glyphs['C'].memory[5] = 0b01000000; font.glyphs['D'].memory[5] = 0b01000010;
   font.glyphs['C'].memory[6] = 0b01000000; font.glyphs['D'].memory[6] = 0b01000010;
   font.glyphs['C'].memory[7] = 0b01000010; font.glyphs['D'].memory[7] = 0b01000010;
   font.glyphs['C'].memory[8] = 0b00111100; font.glyphs['D'].memory[8] = 0b01111100;
   font.glyphs['C'].memory[9] = 0b00000000; font.glyphs['D'].memory[9] = 0b00000000;

   font.glyphs['E'].memory[0] = 0b01111110; font.glyphs['F'].memory[0] = 0b01111110;
   font.glyphs['E'].memory[1] = 0b01000000; font.glyphs['F'].memory[1] = 0b01000000;
   font.glyphs['E'].memory[2] = 0b01000000; font.glyphs['F'].memory[2] = 0b01000000;
   font.glyphs['E'].memory[3] = 0b01000000; font.glyphs['F'].memory[3] = 0b01000000;
   font.glyphs['E'].memory[4] = 0b01111000; font.glyphs['F'].memory[4] = 0b01111000;
   font.glyphs['E'].memory[5] = 0b01000000; font.glyphs['F'].memory[5] = 0b01000000;
   font.glyphs['E'].memory[6] = 0b01000000; font.glyphs['F'].memory[6] = 0b01000000;
   font.glyphs['E'].memory[7] = 0b01000000; font.glyphs['F'].memory[7] = 0b01000000;
   font.glyphs['E'].memory[8] = 0b01111110; font.glyphs['F'].memory[8] = 0b01000000;
   font.glyphs['E'].memory[9] = 0b00000000; font.glyphs['F'].memory[9] = 0b00000000;

   font.glyphs['G'].memory[0] = 0b00111100; font.glyphs['H'].memory[0] = 0b01000010;
   font.glyphs['G'].memory[1] = 0b01000010; font.glyphs['H'].memory[1] = 0b01000010;
   font.glyphs['G'].memory[2] = 0b01000000; font.glyphs['H'].memory[2] = 0b01000010;
   font.glyphs['G'].memory[3] = 0b01000000; font.glyphs['H'].memory[3] = 0b01000010;
   font.glyphs['G'].memory[4] = 0b01001110; font.glyphs['H'].memory[4] = 0b01111110;
   font.glyphs['G'].memory[5] = 0b01000010; font.glyphs['H'].memory[5] = 0b01000010;
   font.glyphs['G'].memory[6] = 0b01000010; font.glyphs['H'].memory[6] = 0b01000010;
   font.glyphs['G'].memory[7] = 0b01000010; font.glyphs['H'].memory[7] = 0b01000010;
   font.glyphs['G'].memory[8] = 0b00111100; font.glyphs['H'].memory[8] = 0b01000010;
   font.glyphs['G'].memory[9] = 0b00000000; font.glyphs['H'].memory[9] = 0b00000000;

   font.glyphs['I'].memory[0] = 0b01111100; font.glyphs['J'].memory[0] = 0b00000100;
   font.glyphs['I'].memory[1] = 0b00010000; font.glyphs['J'].memory[1] = 0b00000100;
   font.glyphs['I'].memory[2] = 0b00010000; font.glyphs['J'].memory[2] = 0b00000100;
   font.glyphs['I'].memory[3] = 0b00010000; font.glyphs['J'].memory[3] = 0b00000100;
   font.glyphs['I'].memory[4] = 0b00010000; font.glyphs['J'].memory[4] = 0b00000100;
   font.glyphs['I'].memory[5] = 0b00010000; font.glyphs['J'].memory[5] = 0b00000100;
   font.glyphs['I'].memory[6] = 0b00010000; font.glyphs['J'].memory[6] = 0b01000100;
   font.glyphs['I'].memory[7] = 0b00010000; font.glyphs['J'].memory[7] = 0b01000100;
   font.glyphs['I'].memory[8] = 0b01111100; font.glyphs['J'].memory[8] = 0b00111000;
   font.glyphs['I'].memory[9] = 0b00000000; font.glyphs['J'].memory[9] = 0b00000000;

   font.glyphs['K'].memory[0] = 0b01000100; font.glyphs['L'].memory[0] = 0b01000000;
   font.glyphs['K'].memory[1] = 0b01001000; font.glyphs['L'].memory[1] = 0b01000000;
   font.glyphs['K'].memory[2] = 0b01001000; font.glyphs['L'].memory[2] = 0b01000000;
   font.glyphs['K'].memory[3] = 0b01010000; font.glyphs['L'].memory[3] = 0b01000000;
   font.glyphs['K'].memory[4] = 0b01110000; font.glyphs['L'].memory[4] = 0b01000000;
   font.glyphs['K'].memory[5] = 0b01001000; font.glyphs['L'].memory[5] = 0b01000000;
   font.glyphs['K'].memory[6] = 0b01001000; font.glyphs['L'].memory[6] = 0b01000000;
   font.glyphs['K'].memory[7] = 0b01000100; font.glyphs['L'].memory[7] = 0b01000000;
   font.glyphs['K'].memory[8] = 0b01000100; font.glyphs['L'].memory[8] = 0b01111100;
   font.glyphs['K'].memory[9] = 0b00000000; font.glyphs['L'].memory[9] = 0b00000000;

   font.glyphs['M'].memory[0] = 0b10000010; font.glyphs['N'].memory[0] = 0b01000010;
   font.glyphs['M'].memory[1] = 0b11000110; font.glyphs['N'].memory[1] = 0b01100010;
   font.glyphs['M'].memory[2] = 0b10101010; font.glyphs['N'].memory[2] = 0b01010010;
   font.glyphs['M'].memory[3] = 0b10010010; font.glyphs['N'].memory[3] = 0b01001010;
   font.glyphs['M'].memory[4] = 0b10000010; font.glyphs['N'].memory[4] = 0b01000110;
   font.glyphs['M'].memory[5] = 0b10000010; font.glyphs['N'].memory[5] = 0b01000010;
   font.glyphs['M'].memory[6] = 0b10000010; font.glyphs['N'].memory[6] = 0b01000010;
   font.glyphs['M'].memory[7] = 0b10000010; font.glyphs['N'].memory[7] = 0b01000010;
   font.glyphs['M'].memory[8] = 0b10000010; font.glyphs['N'].memory[8] = 0b01000010;
   font.glyphs['M'].memory[9] = 0b00000000; font.glyphs['N'].memory[9] = 0b00000000;

   font.glyphs['O'].memory[0] = 0b00111100; font.glyphs['P'].memory[0] = 0b01111100;
   font.glyphs['O'].memory[1] = 0b01000010; font.glyphs['P'].memory[1] = 0b01000010;
   font.glyphs['O'].memory[2] = 0b01000010; font.glyphs['P'].memory[2] = 0b01000010;
   font.glyphs['O'].memory[3] = 0b01000010; font.glyphs['P'].memory[3] = 0b01000010;
   font.glyphs['O'].memory[4] = 0b01000010; font.glyphs['P'].memory[4] = 0b01111100;
   font.glyphs['O'].memory[5] = 0b01000010; font.glyphs['P'].memory[5] = 0b01000000;
   font.glyphs['O'].memory[6] = 0b01000010; font.glyphs['P'].memory[6] = 0b01000000;
   font.glyphs['O'].memory[7] = 0b01000010; font.glyphs['P'].memory[7] = 0b01000000;
   font.glyphs['O'].memory[8] = 0b00111100; font.glyphs['P'].memory[8] = 0b01000000;
   font.glyphs['O'].memory[9] = 0b00000000; font.glyphs['P'].memory[9] = 0b00000000;

   font.glyphs['Q'].memory[0] = 0b00111100; font.glyphs['R'].memory[0] = 0b01111100;
   font.glyphs['Q'].memory[1] = 0b01000010; font.glyphs['R'].memory[1] = 0b01000010;
   font.glyphs['Q'].memory[2] = 0b01000010; font.glyphs['R'].memory[2] = 0b01000010;
   font.glyphs['Q'].memory[3] = 0b01000010; font.glyphs['R'].memory[3] = 0b01000010;
   font.glyphs['Q'].memory[4] = 0b01000010; font.glyphs['R'].memory[4] = 0b01111100;
   font.glyphs['Q'].memory[5] = 0b01000010; font.glyphs['R'].memory[5] = 0b01000100;
   font.glyphs['Q'].memory[6] = 0b01000010; font.glyphs['R'].memory[6] = 0b01000010;
   font.glyphs['Q'].memory[7] = 0b01001010; font.glyphs['R'].memory[7] = 0b01000010;
   font.glyphs['Q'].memory[8] = 0b00101000; font.glyphs['R'].memory[8] = 0b01000010;
   font.glyphs['Q'].memory[9] = 0b00000110; font.glyphs['R'].memory[9] = 0b00000000;

   font.glyphs['S'].memory[0] = 0b00111100; font.glyphs['T'].memory[0] = 0b01111111;
   font.glyphs['S'].memory[1] = 0b01000010; font.glyphs['T'].memory[1] = 0b00001000;
   font.glyphs['S'].memory[2] = 0b01000000; font.glyphs['T'].memory[2] = 0b00001000;
   font.glyphs['S'].memory[3] = 0b01000000; font.glyphs['T'].memory[3] = 0b00001000;
   font.glyphs['S'].memory[4] = 0b00111100; font.glyphs['T'].memory[4] = 0b00001000;
   font.glyphs['S'].memory[5] = 0b00000010; font.glyphs['T'].memory[5] = 0b00001000;
   font.glyphs['S'].memory[6] = 0b00000010; font.glyphs['T'].memory[6] = 0b00001000;
   font.glyphs['S'].memory[7] = 0b01000010; font.glyphs['T'].memory[7] = 0b00001000;
   font.glyphs['S'].memory[8] = 0b00111100; font.glyphs['T'].memory[8] = 0b00001000;
   font.glyphs['S'].memory[9] = 0b00000000; font.glyphs['T'].memory[9] = 0b00000000;

   font.glyphs['U'].memory[0] = 0b01000010; font.glyphs['V'].memory[0] = 0b01000100;
   font.glyphs['U'].memory[1] = 0b01000010; font.glyphs['V'].memory[1] = 0b01000100;
   font.glyphs['U'].memory[2] = 0b01000010; font.glyphs['V'].memory[2] = 0b01000100;
   font.glyphs['U'].memory[3] = 0b01000010; font.glyphs['V'].memory[3] = 0b01000100;
   font.glyphs['U'].memory[4] = 0b01000010; font.glyphs['V'].memory[4] = 0b01000100;
   font.glyphs['U'].memory[5] = 0b01000010; font.glyphs['V'].memory[5] = 0b00101000;
   font.glyphs['U'].memory[6] = 0b01000010; font.glyphs['V'].memory[6] = 0b00101000;
   font.glyphs['U'].memory[7] = 0b01000010; font.glyphs['V'].memory[7] = 0b00101000;
   font.glyphs['U'].memory[8] = 0b00111100; font.glyphs['V'].memory[8] = 0b00010000;
   font.glyphs['U'].memory[9] = 0b00000000; font.glyphs['V'].memory[9] = 0b00000000;

   font.glyphs['W'].memory[0] = 0b10000010; font.glyphs['X'].memory[0] = 0b01000100;
   font.glyphs['W'].memory[1] = 0b10000010; font.glyphs['X'].memory[1] = 0b01000100;
   font.glyphs['W'].memory[2] = 0b10000010; font.glyphs['X'].memory[2] = 0b01000100;
   font.glyphs['W'].memory[3] = 0b10000010; font.glyphs['X'].memory[3] = 0b00101000;
   font.glyphs['W'].memory[4] = 0b10000010; font.glyphs['X'].memory[4] = 0b00010000;
   font.glyphs['W'].memory[5] = 0b10010010; font.glyphs['X'].memory[5] = 0b00101000;
   font.glyphs['W'].memory[6] = 0b10101010; font.glyphs['X'].memory[6] = 0b01000100;
   font.glyphs['W'].memory[7] = 0b11000110; font.glyphs['X'].memory[7] = 0b01000100;
   font.glyphs['W'].memory[8] = 0b10000010; font.glyphs['X'].memory[8] = 0b01000100;
   font.glyphs['W'].memory[9] = 0b00000000; font.glyphs['X'].memory[9] = 0b00000000;

   font.glyphs['Y'].memory[0] = 0b01000100; font.glyphs['Z'].memory[0] = 0b11111110;
   font.glyphs['Y'].memory[1] = 0b01000100; font.glyphs['Z'].memory[1] = 0b00000010;
   font.glyphs['Y'].memory[2] = 0b01000100; font.glyphs['Z'].memory[2] = 0b00000100;
   font.glyphs['Y'].memory[3] = 0b00101000; font.glyphs['Z'].memory[3] = 0b00001000;
   font.glyphs['Y'].memory[4] = 0b00010000; font.glyphs['Z'].memory[4] = 0b01111100;
   font.glyphs['Y'].memory[5] = 0b00010000; font.glyphs['Z'].memory[5] = 0b00100000;
   font.glyphs['Y'].memory[6] = 0b00010000; font.glyphs['Z'].memory[6] = 0b01000000;
   font.glyphs['Y'].memory[7] = 0b00010000; font.glyphs['Z'].memory[7] = 0b10000000;
   font.glyphs['Y'].memory[8] = 0b00010000; font.glyphs['Z'].memory[8] = 0b11111110;
   font.glyphs['Y'].memory[9] = 0b00000000; font.glyphs['Z'].memory[9] = 0b00000000;

   /////////////////////////////////////////////////////////////////////////////////

   font.glyphs['a'].memory[0] = 0b00000000; font.glyphs['b'].memory[0] = 0b01000000;
   font.glyphs['a'].memory[1] = 0b00000000; font.glyphs['b'].memory[1] = 0b01000000;
   font.glyphs['a'].memory[2] = 0b00000000; font.glyphs['b'].memory[2] = 0b01000000;
   font.glyphs['a'].memory[3] = 0b01111100; font.glyphs['b'].memory[3] = 0b01111100;
   font.glyphs['a'].memory[4] = 0b00000010; font.glyphs['b'].memory[4] = 0b01000010;
   font.glyphs['a'].memory[5] = 0b01111110; font.glyphs['b'].memory[5] = 0b01000010;
   font.glyphs['a'].memory[6] = 0b10000010; font.glyphs['b'].memory[6] = 0b01000010;
   font.glyphs['a'].memory[7] = 0b10000010; font.glyphs['b'].memory[7] = 0b01000010;
   font.glyphs['a'].memory[8] = 0b01111110; font.glyphs['b'].memory[8] = 0b01111100;
   font.glyphs['a'].memory[9] = 0b00000000; font.glyphs['b'].memory[9] = 0b00000000;

   font.glyphs['c'].memory[0] = 0b00000000; font.glyphs['d'].memory[0] = 0b00000010;
   font.glyphs['c'].memory[1] = 0b00000000; font.glyphs['d'].memory[1] = 0b00000010;
   font.glyphs['c'].memory[2] = 0b00000000; font.glyphs['d'].memory[2] = 0b00000010;
   font.glyphs['c'].memory[3] = 0b00111100; font.glyphs['d'].memory[3] = 0b00111110;
   font.glyphs['c'].memory[4] = 0b01000010; font.glyphs['d'].memory[4] = 0b01000010;
   font.glyphs['c'].memory[5] = 0b01000000; font.glyphs['d'].memory[5] = 0b01000010;
   font.glyphs['c'].memory[6] = 0b01000000; font.glyphs['d'].memory[6] = 0b01000010;
   font.glyphs['c'].memory[7] = 0b01000010; font.glyphs['d'].memory[7] = 0b01000010;
   font.glyphs['c'].memory[8] = 0b00111100; font.glyphs['d'].memory[8] = 0b00111110;
   font.glyphs['c'].memory[9] = 0b00000000; font.glyphs['d'].memory[9] = 0b00000000;

   font.glyphs['e'].memory[0] = 0b00000000; font.glyphs['f'].memory[0] = 0b00001110;
   font.glyphs['e'].memory[1] = 0b00000000; font.glyphs['f'].memory[1] = 0b00010000;
   font.glyphs['e'].memory[2] = 0b00000000; font.glyphs['f'].memory[2] = 0b00010000;
   font.glyphs['e'].memory[3] = 0b00111100; font.glyphs['f'].memory[3] = 0b00111100;
   font.glyphs['e'].memory[4] = 0b01000010; font.glyphs['f'].memory[4] = 0b00010000;
   font.glyphs['e'].memory[5] = 0b01111110; font.glyphs['f'].memory[5] = 0b00010000;
   font.glyphs['e'].memory[6] = 0b01000000; font.glyphs['f'].memory[6] = 0b00010000;
   font.glyphs['e'].memory[7] = 0b01000010; font.glyphs['f'].memory[7] = 0b00010000;
   font.glyphs['e'].memory[8] = 0b00111100; font.glyphs['f'].memory[8] = 0b00010000;
   font.glyphs['e'].memory[9] = 0b00000000; font.glyphs['f'].memory[9] = 0b00000000;

   font.glyphs['g'].memory[0] = 0b00000000; font.glyphs['h'].memory[0] = 0b01000000;
   font.glyphs['g'].memory[1] = 0b00000000; font.glyphs['h'].memory[1] = 0b01000000;
   font.glyphs['g'].memory[2] = 0b00000000; font.glyphs['h'].memory[2] = 0b01000000;
   font.glyphs['g'].memory[3] = 0b00111110; font.glyphs['h'].memory[3] = 0b01111100;
   font.glyphs['g'].memory[4] = 0b01000010; font.glyphs['h'].memory[4] = 0b01000010;
   font.glyphs['g'].memory[5] = 0b01000010; font.glyphs['h'].memory[5] = 0b01000010;
   font.glyphs['g'].memory[6] = 0b01000010; font.glyphs['h'].memory[6] = 0b01000010;
   font.glyphs['g'].memory[7] = 0b00111110; font.glyphs['h'].memory[7] = 0b01000010;
   font.glyphs['g'].memory[8] = 0b00000010; font.glyphs['h'].memory[8] = 0b01000010;
   font.glyphs['g'].memory[9] = 0b00111100; font.glyphs['h'].memory[9] = 0b00000000;

   font.glyphs['i'].memory[0] = 0b00000000; font.glyphs['j'].memory[0] = 0b00000000;
   font.glyphs['i'].memory[1] = 0b00010000; font.glyphs['j'].memory[1] = 0b00000100;
   font.glyphs['i'].memory[2] = 0b00000000; font.glyphs['j'].memory[2] = 0b00000000;
   font.glyphs['i'].memory[3] = 0b00110000; font.glyphs['j'].memory[3] = 0b00000100;
   font.glyphs['i'].memory[4] = 0b00010000; font.glyphs['j'].memory[4] = 0b00000100;
   font.glyphs['i'].memory[5] = 0b00010000; font.glyphs['j'].memory[5] = 0b00000100;
   font.glyphs['i'].memory[6] = 0b00010000; font.glyphs['j'].memory[6] = 0b00000100;
   font.glyphs['i'].memory[7] = 0b00010000; font.glyphs['j'].memory[7] = 0b00000100;
   font.glyphs['i'].memory[8] = 0b01111100; font.glyphs['j'].memory[8] = 0b00000100;
   font.glyphs['i'].memory[9] = 0b00000000; font.glyphs['j'].memory[9] = 0b00111000;

   font.glyphs['k'].memory[0] = 0b00100000; font.glyphs['l'].memory[0] = 0b01110000;
   font.glyphs['k'].memory[1] = 0b00100000; font.glyphs['l'].memory[1] = 0b00010000;
   font.glyphs['k'].memory[2] = 0b00100000; font.glyphs['l'].memory[2] = 0b00010000;
   font.glyphs['k'].memory[3] = 0b00100010; font.glyphs['l'].memory[3] = 0b00010000;
   font.glyphs['k'].memory[4] = 0b00100100; font.glyphs['l'].memory[4] = 0b00010000;
   font.glyphs['k'].memory[5] = 0b00111000; font.glyphs['l'].memory[5] = 0b00010000;
   font.glyphs['k'].memory[6] = 0b00100100; font.glyphs['l'].memory[6] = 0b00010000;
   font.glyphs['k'].memory[7] = 0b00100010; font.glyphs['l'].memory[7] = 0b00010000;
   font.glyphs['k'].memory[8] = 0b00100010; font.glyphs['l'].memory[8] = 0b00011100;
   font.glyphs['k'].memory[9] = 0b00000000; font.glyphs['l'].memory[9] = 0b00000000;

   font.glyphs['m'].memory[0] = 0b00000000; font.glyphs['n'].memory[0] = 0b00000000;
   font.glyphs['m'].memory[1] = 0b00000000; font.glyphs['n'].memory[1] = 0b00000000;
   font.glyphs['m'].memory[2] = 0b00000000; font.glyphs['n'].memory[2] = 0b00000000;
   font.glyphs['m'].memory[3] = 0b11101100; font.glyphs['n'].memory[3] = 0b01111000;
   font.glyphs['m'].memory[4] = 0b10010010; font.glyphs['n'].memory[4] = 0b01000100;
   font.glyphs['m'].memory[5] = 0b10010010; font.glyphs['n'].memory[5] = 0b01000100;
   font.glyphs['m'].memory[6] = 0b10010010; font.glyphs['n'].memory[6] = 0b01000100;
   font.glyphs['m'].memory[7] = 0b10010010; font.glyphs['n'].memory[7] = 0b01000100;
   font.glyphs['m'].memory[8] = 0b10010010; font.glyphs['n'].memory[8] = 0b01000100;
   font.glyphs['m'].memory[9] = 0b00000000; font.glyphs['n'].memory[9] = 0b00000000;

   font.glyphs['o'].memory[0] = 0b00000000; font.glyphs['p'].memory[0] = 0b00000000;
   font.glyphs['o'].memory[1] = 0b00000000; font.glyphs['p'].memory[1] = 0b00000000;
   font.glyphs['o'].memory[2] = 0b00000000; font.glyphs['p'].memory[2] = 0b00000000;
   font.glyphs['o'].memory[3] = 0b00111100; font.glyphs['p'].memory[3] = 0b01111100;
   font.glyphs['o'].memory[4] = 0b01000010; font.glyphs['p'].memory[4] = 0b01000010;
   font.glyphs['o'].memory[5] = 0b01000010; font.glyphs['p'].memory[5] = 0b01000010;
   font.glyphs['o'].memory[6] = 0b01000010; font.glyphs['p'].memory[6] = 0b01000010;
   font.glyphs['o'].memory[7] = 0b01000010; font.glyphs['p'].memory[7] = 0b01111100;
   font.glyphs['o'].memory[8] = 0b00111100; font.glyphs['p'].memory[8] = 0b01000000;
   font.glyphs['o'].memory[9] = 0b00000000; font.glyphs['p'].memory[9] = 0b01000000;

   font.glyphs['q'].memory[0] = 0b00000000; font.glyphs['r'].memory[0] = 0b00000000;
   font.glyphs['q'].memory[1] = 0b00000000; font.glyphs['r'].memory[1] = 0b00000000;
   font.glyphs['q'].memory[2] = 0b00000000; font.glyphs['r'].memory[2] = 0b00000000;
   font.glyphs['q'].memory[3] = 0b00111110; font.glyphs['r'].memory[3] = 0b01111100;
   font.glyphs['q'].memory[4] = 0b01000010; font.glyphs['r'].memory[4] = 0b01000010;
   font.glyphs['q'].memory[5] = 0b01000010; font.glyphs['r'].memory[5] = 0b01000000;
   font.glyphs['q'].memory[6] = 0b01000010; font.glyphs['r'].memory[6] = 0b01000000;
   font.glyphs['q'].memory[7] = 0b00111110; font.glyphs['r'].memory[7] = 0b01000000;
   font.glyphs['q'].memory[8] = 0b00000010; font.glyphs['r'].memory[8] = 0b01000000;
   font.glyphs['q'].memory[9] = 0b00000010; font.glyphs['r'].memory[9] = 0b00000000;

   font.glyphs['s'].memory[0] = 0b00000000; font.glyphs['t'].memory[0] = 0b00000000;
   font.glyphs['s'].memory[1] = 0b00000000; font.glyphs['t'].memory[1] = 0b00000000;
   font.glyphs['s'].memory[2] = 0b00000000; font.glyphs['t'].memory[2] = 0b00010000;
   font.glyphs['s'].memory[3] = 0b00111100; font.glyphs['t'].memory[3] = 0b01111110;
   font.glyphs['s'].memory[4] = 0b01000010; font.glyphs['t'].memory[4] = 0b00010000;
   font.glyphs['s'].memory[5] = 0b00110000; font.glyphs['t'].memory[5] = 0b00010000;
   font.glyphs['s'].memory[6] = 0b00001100; font.glyphs['t'].memory[6] = 0b00010000;
   font.glyphs['s'].memory[7] = 0b01000010; font.glyphs['t'].memory[7] = 0b00010000;
   font.glyphs['s'].memory[8] = 0b00111100; font.glyphs['t'].memory[8] = 0b00001110;
   font.glyphs['s'].memory[9] = 0b00000000; font.glyphs['t'].memory[9] = 0b00000000;

   font.glyphs['u'].memory[0] = 0b00000000; font.glyphs['v'].memory[0] = 0b00000000;
   font.glyphs['u'].memory[1] = 0b00000000; font.glyphs['v'].memory[1] = 0b00000000;
   font.glyphs['u'].memory[2] = 0b00000000; font.glyphs['v'].memory[2] = 0b00000000;
   font.glyphs['u'].memory[3] = 0b01000100; font.glyphs['v'].memory[3] = 0b01000100;
   font.glyphs['u'].memory[4] = 0b01000100; font.glyphs['v'].memory[4] = 0b01000100;
   font.glyphs['u'].memory[5] = 0b01000100; font.glyphs['v'].memory[5] = 0b01000100;
   font.glyphs['u'].memory[6] = 0b01000100; font.glyphs['v'].memory[6] = 0b00101000;
   font.glyphs['u'].memory[7] = 0b01000100; font.glyphs['v'].memory[7] = 0b00101000;
   font.glyphs['u'].memory[8] = 0b00111100; font.glyphs['v'].memory[8] = 0b00010000;
   font.glyphs['u'].memory[9] = 0b00000000; font.glyphs['v'].memory[9] = 0b00000000;

   font.glyphs['w'].memory[0] = 0b00000000; font.glyphs['x'].memory[0] = 0b00000000;
   font.glyphs['w'].memory[1] = 0b00000000; font.glyphs['x'].memory[1] = 0b00000000;
   font.glyphs['w'].memory[2] = 0b00000000; font.glyphs['x'].memory[2] = 0b00000000;
   font.glyphs['w'].memory[3] = 0b01000001; font.glyphs['x'].memory[3] = 0b01000100;
   font.glyphs['w'].memory[4] = 0b01001001; font.glyphs['x'].memory[4] = 0b00101000;
   font.glyphs['w'].memory[5] = 0b01001001; font.glyphs['x'].memory[5] = 0b00010000;
   font.glyphs['w'].memory[6] = 0b01001001; font.glyphs['x'].memory[6] = 0b00010000;
   font.glyphs['w'].memory[7] = 0b01010101; font.glyphs['x'].memory[7] = 0b00101000;
   font.glyphs['w'].memory[8] = 0b00100010; font.glyphs['x'].memory[8] = 0b01000100;
   font.glyphs['w'].memory[9] = 0b00000000; font.glyphs['x'].memory[9] = 0b00000000;

   font.glyphs['y'].memory[0] = 0b00000000; font.glyphs['z'].memory[0] = 0b00000000;
   font.glyphs['y'].memory[1] = 0b00000000; font.glyphs['z'].memory[1] = 0b00000000;
   font.glyphs['y'].memory[2] = 0b00000000; font.glyphs['z'].memory[2] = 0b00000000;
   font.glyphs['y'].memory[3] = 0b01000010; font.glyphs['z'].memory[3] = 0b01111110;
   font.glyphs['y'].memory[4] = 0b01000010; font.glyphs['z'].memory[4] = 0b00000100;
   font.glyphs['y'].memory[5] = 0b01000010; font.glyphs['z'].memory[5] = 0b00001000;
   font.glyphs['y'].memory[6] = 0b00111110; font.glyphs['z'].memory[6] = 0b00010000;
   font.glyphs['y'].memory[7] = 0b00000010; font.glyphs['z'].memory[7] = 0b00100000;
   font.glyphs['y'].memory[8] = 0b00000010; font.glyphs['z'].memory[8] = 0b01111110;
   font.glyphs['y'].memory[9] = 0b00111100; font.glyphs['z'].memory[9] = 0b00000000;

   /////////////////////////////////////////////////////////////////////////////////

   font.glyphs['!'].memory[0] = 0b00010000; font.glyphs['"'].memory[0] = 0b00000000;
   font.glyphs['!'].memory[1] = 0b00010000; font.glyphs['"'].memory[1] = 0b00100100;
   font.glyphs['!'].memory[2] = 0b00010000; font.glyphs['"'].memory[2] = 0b00100100;
   font.glyphs['!'].memory[3] = 0b00010000; font.glyphs['"'].memory[3] = 0b00100100;
   font.glyphs['!'].memory[4] = 0b00010000; font.glyphs['"'].memory[4] = 0b00000000;
   font.glyphs['!'].memory[5] = 0b00010000; font.glyphs['"'].memory[5] = 0b00000000;
   font.glyphs['!'].memory[6] = 0b00000000; font.glyphs['"'].memory[6] = 0b00000000;
   font.glyphs['!'].memory[7] = 0b00000000; font.glyphs['"'].memory[7] = 0b00000000;
   font.glyphs['!'].memory[8] = 0b00010000; font.glyphs['"'].memory[8] = 0b00000000;
   font.glyphs['!'].memory[9] = 0b00000000; font.glyphs['"'].memory[9] = 0b00000000;

   font.glyphs['#'].memory[0] = 0b00000000; font.glyphs['$'].memory[0] = 0b00010000;
   font.glyphs['#'].memory[1] = 0b00100100; font.glyphs['$'].memory[1] = 0b00111100;
   font.glyphs['#'].memory[2] = 0b01111110; font.glyphs['$'].memory[2] = 0b01000010;
   font.glyphs['#'].memory[3] = 0b00100100; font.glyphs['$'].memory[3] = 0b01000000;
   font.glyphs['#'].memory[4] = 0b00100100; font.glyphs['$'].memory[4] = 0b00111100;
   font.glyphs['#'].memory[5] = 0b00100100; font.glyphs['$'].memory[5] = 0b00000010;
   font.glyphs['#'].memory[6] = 0b01111110; font.glyphs['$'].memory[6] = 0b01000010;
   font.glyphs['#'].memory[7] = 0b00100100; font.glyphs['$'].memory[7] = 0b00111100;
   font.glyphs['#'].memory[8] = 0b00000000; font.glyphs['$'].memory[8] = 0b00001000;
   font.glyphs['#'].memory[9] = 0b00000000; font.glyphs['$'].memory[9] = 0b00000000;

   font.glyphs['%'].memory[0] = 0b00000000; font.glyphs['&'].memory[0] = 0b00111000;
   font.glyphs['%'].memory[1] = 0b01100010; font.glyphs['&'].memory[1] = 0b01000100;
   font.glyphs['%'].memory[2] = 0b01100100; font.glyphs['&'].memory[2] = 0b01000000;
   font.glyphs['%'].memory[3] = 0b00001000; font.glyphs['&'].memory[3] = 0b00100000;
   font.glyphs['%'].memory[4] = 0b00010000; font.glyphs['&'].memory[4] = 0b00110010;
   font.glyphs['%'].memory[5] = 0b00100000; font.glyphs['&'].memory[5] = 0b01001100;
   font.glyphs['%'].memory[6] = 0b01001100; font.glyphs['&'].memory[6] = 0b01000110;
   font.glyphs['%'].memory[7] = 0b00001100; font.glyphs['&'].memory[7] = 0b01000010;
   font.glyphs['%'].memory[8] = 0b00000000; font.glyphs['&'].memory[8] = 0b00111101;
   font.glyphs['%'].memory[9] = 0b00000000; font.glyphs['&'].memory[9] = 0b00000000;

   font.glyphs['\''].memory[0] = 0b00000000; font.glyphs['('].memory[0] = 0b00001100;
   font.glyphs['\''].memory[1] = 0b00010000; font.glyphs['('].memory[1] = 0b00010000;
   font.glyphs['\''].memory[2] = 0b00010000; font.glyphs['('].memory[2] = 0b00100000;
   font.glyphs['\''].memory[3] = 0b00010000; font.glyphs['('].memory[3] = 0b00100000;
   font.glyphs['\''].memory[4] = 0b00000000; font.glyphs['('].memory[4] = 0b00100000;
   font.glyphs['\''].memory[5] = 0b00000000; font.glyphs['('].memory[5] = 0b00100000;
   font.glyphs['\''].memory[6] = 0b00000000; font.glyphs['('].memory[6] = 0b00100000;
   font.glyphs['\''].memory[7] = 0b00000000; font.glyphs['('].memory[7] = 0b00010000;
   font.glyphs['\''].memory[8] = 0b00000000; font.glyphs['('].memory[8] = 0b00001100;
   font.glyphs['\''].memory[9] = 0b00000000; font.glyphs['('].memory[9] = 0b00000000;

   font.glyphs[')'].memory[0] = 0b00110000; font.glyphs['*'].memory[0] = 0b00000000;
   font.glyphs[')'].memory[1] = 0b00001000; font.glyphs['*'].memory[1] = 0b00000000;
   font.glyphs[')'].memory[2] = 0b00000100; font.glyphs['*'].memory[2] = 0b00101000;
   font.glyphs[')'].memory[3] = 0b00000100; font.glyphs['*'].memory[3] = 0b00010000;
   font.glyphs[')'].memory[4] = 0b00000100; font.glyphs['*'].memory[4] = 0b01111100;
   font.glyphs[')'].memory[5] = 0b00000100; font.glyphs['*'].memory[5] = 0b00010000;
   font.glyphs[')'].memory[6] = 0b00000100; font.glyphs['*'].memory[6] = 0b00101000;
   font.glyphs[')'].memory[7] = 0b00001000; font.glyphs['*'].memory[7] = 0b00000000;
   font.glyphs[')'].memory[8] = 0b00110000; font.glyphs['*'].memory[8] = 0b00000000;
   font.glyphs[')'].memory[9] = 0b00000000; font.glyphs['*'].memory[9] = 0b00000000;

   font.glyphs['+'].memory[0] = 0b00000000; font.glyphs[','].memory[0] = 0b00000000;
   font.glyphs['+'].memory[1] = 0b00000000; font.glyphs[','].memory[1] = 0b00000000;
   font.glyphs['+'].memory[2] = 0b00010000; font.glyphs[','].memory[2] = 0b00000000;
   font.glyphs['+'].memory[3] = 0b00010000; font.glyphs[','].memory[3] = 0b00000000;
   font.glyphs['+'].memory[4] = 0b01111100; font.glyphs[','].memory[4] = 0b00000000;
   font.glyphs['+'].memory[5] = 0b00010000; font.glyphs[','].memory[5] = 0b00000000;
   font.glyphs['+'].memory[6] = 0b00010000; font.glyphs[','].memory[6] = 0b00011000;
   font.glyphs['+'].memory[7] = 0b00000000; font.glyphs[','].memory[7] = 0b00011000;
   font.glyphs['+'].memory[8] = 0b00000000; font.glyphs[','].memory[8] = 0b00001000;
   font.glyphs['+'].memory[9] = 0b00000000; font.glyphs[','].memory[9] = 0b00010000;

   font.glyphs['-'].memory[0] = 0b00000000; font.glyphs['.'].memory[0] = 0b00000000;
   font.glyphs['-'].memory[1] = 0b00000000; font.glyphs['.'].memory[1] = 0b00000000;
   font.glyphs['-'].memory[2] = 0b00000000; font.glyphs['.'].memory[2] = 0b00000000;
   font.glyphs['-'].memory[3] = 0b00000000; font.glyphs['.'].memory[3] = 0b00000000;
   font.glyphs['-'].memory[4] = 0b01111110; font.glyphs['.'].memory[4] = 0b00000000;
   font.glyphs['-'].memory[5] = 0b00000000; font.glyphs['.'].memory[5] = 0b00000000;
   font.glyphs['-'].memory[6] = 0b00000000; font.glyphs['.'].memory[6] = 0b00000000;
   font.glyphs['-'].memory[7] = 0b00000000; font.glyphs['.'].memory[7] = 0b00011000;
   font.glyphs['-'].memory[8] = 0b00000000; font.glyphs['.'].memory[8] = 0b00011000;
   font.glyphs['-'].memory[9] = 0b00000000; font.glyphs['.'].memory[9] = 0b00000000;

   font.glyphs['/'].memory[0] = 0b00000100; font.glyphs[':'].memory[0] = 0b00000000;
   font.glyphs['/'].memory[1] = 0b00000100; font.glyphs[':'].memory[1] = 0b00000000;
   font.glyphs['/'].memory[2] = 0b00001000; font.glyphs[':'].memory[2] = 0b00000000;
   font.glyphs['/'].memory[3] = 0b00001000; font.glyphs[':'].memory[3] = 0b00011000;
   font.glyphs['/'].memory[4] = 0b00010000; font.glyphs[':'].memory[4] = 0b00011000;
   font.glyphs['/'].memory[5] = 0b00010000; font.glyphs[':'].memory[5] = 0b00000000;
   font.glyphs['/'].memory[6] = 0b00100000; font.glyphs[':'].memory[6] = 0b00011000;
   font.glyphs['/'].memory[7] = 0b00100000; font.glyphs[':'].memory[7] = 0b00011000;
   font.glyphs['/'].memory[8] = 0b01000000; font.glyphs[':'].memory[8] = 0b00000000;
   font.glyphs['/'].memory[9] = 0b01000000; font.glyphs[':'].memory[9] = 0b00000000;

   font.glyphs[';'].memory[0] = 0b00000000; font.glyphs['<'].memory[0] = 0b00000000;
   font.glyphs[';'].memory[1] = 0b00000000; font.glyphs['<'].memory[1] = 0b00001000;
   font.glyphs[';'].memory[2] = 0b00000000; font.glyphs['<'].memory[2] = 0b00010000;
   font.glyphs[';'].memory[3] = 0b00011000; font.glyphs['<'].memory[3] = 0b00100000;
   font.glyphs[';'].memory[4] = 0b00011000; font.glyphs['<'].memory[4] = 0b01000000;
   font.glyphs[';'].memory[5] = 0b00000000; font.glyphs['<'].memory[5] = 0b00100000;
   font.glyphs[';'].memory[6] = 0b00011000; font.glyphs['<'].memory[6] = 0b00010000;
   font.glyphs[';'].memory[7] = 0b00011000; font.glyphs['<'].memory[7] = 0b00001000;
   font.glyphs[';'].memory[8] = 0b00001000; font.glyphs['<'].memory[8] = 0b00000000;
   font.glyphs[';'].memory[9] = 0b00010000; font.glyphs['<'].memory[9] = 0b00000000;

   font.glyphs['='].memory[0] = 0b00000000; font.glyphs['>'].memory[0] = 0b00000000;
   font.glyphs['='].memory[1] = 0b00000000; font.glyphs['>'].memory[1] = 0b00010000;
   font.glyphs['='].memory[2] = 0b00000000; font.glyphs['>'].memory[2] = 0b00001000;
   font.glyphs['='].memory[3] = 0b01111110; font.glyphs['>'].memory[3] = 0b00000100;
   font.glyphs['='].memory[4] = 0b00000000; font.glyphs['>'].memory[4] = 0b00000010;
   font.glyphs['='].memory[5] = 0b00000000; font.glyphs['>'].memory[5] = 0b00000100;
   font.glyphs['='].memory[6] = 0b01111110; font.glyphs['>'].memory[6] = 0b00001000;
   font.glyphs['='].memory[7] = 0b00000000; font.glyphs['>'].memory[7] = 0b00010000;
   font.glyphs['='].memory[8] = 0b00000000; font.glyphs['>'].memory[8] = 0b00000000;
   font.glyphs['='].memory[9] = 0b00000000; font.glyphs['>'].memory[9] = 0b00000000;

   font.glyphs['?'].memory[0] = 0b00111100; font.glyphs['@'].memory[0] = 0b00111100;
   font.glyphs['?'].memory[1] = 0b01000010; font.glyphs['@'].memory[1] = 0b01000010;
   font.glyphs['?'].memory[2] = 0b00000010; font.glyphs['@'].memory[2] = 0b01001110;
   font.glyphs['?'].memory[3] = 0b00000010; font.glyphs['@'].memory[3] = 0b01010010;
   font.glyphs['?'].memory[4] = 0b00011100; font.glyphs['@'].memory[4] = 0b01010010;
   font.glyphs['?'].memory[5] = 0b00000000; font.glyphs['@'].memory[5] = 0b01010010;
   font.glyphs['?'].memory[6] = 0b00011000; font.glyphs['@'].memory[6] = 0b01001100;
   font.glyphs['?'].memory[7] = 0b00011000; font.glyphs['@'].memory[7] = 0b01000000;
   font.glyphs['?'].memory[8] = 0b00000000; font.glyphs['@'].memory[8] = 0b00111100;
   font.glyphs['?'].memory[9] = 0b00000000; font.glyphs['@'].memory[9] = 0b00000000;

   font.glyphs['['].memory[0] = 0b00111100; font.glyphs['\\'].memory[0] = 0b01000000;
   font.glyphs['['].memory[1] = 0b00100000; font.glyphs['\\'].memory[1] = 0b01000000;
   font.glyphs['['].memory[2] = 0b00100000; font.glyphs['\\'].memory[2] = 0b00100000;
   font.glyphs['['].memory[3] = 0b00100000; font.glyphs['\\'].memory[3] = 0b00100000;
   font.glyphs['['].memory[4] = 0b00100000; font.glyphs['\\'].memory[4] = 0b00010000;
   font.glyphs['['].memory[5] = 0b00100000; font.glyphs['\\'].memory[5] = 0b00010000;
   font.glyphs['['].memory[6] = 0b00100000; font.glyphs['\\'].memory[6] = 0b00001000;
   font.glyphs['['].memory[7] = 0b00100000; font.glyphs['\\'].memory[7] = 0b00001000;
   font.glyphs['['].memory[8] = 0b00111100; font.glyphs['\\'].memory[8] = 0b00000100;
   font.glyphs['['].memory[9] = 0b00000000; font.glyphs['\\'].memory[9] = 0b00000100;

   font.glyphs[']'].memory[0] = 0b00111100; font.glyphs['^'].memory[0] = 0b00010000;
   font.glyphs[']'].memory[1] = 0b00000100; font.glyphs['^'].memory[1] = 0b00101000;
   font.glyphs[']'].memory[2] = 0b00000100; font.glyphs['^'].memory[2] = 0b01000100;
   font.glyphs[']'].memory[3] = 0b00000100; font.glyphs['^'].memory[3] = 0b10000010;
   font.glyphs[']'].memory[4] = 0b00000100; font.glyphs['^'].memory[4] = 0b00000000;
   font.glyphs[']'].memory[5] = 0b00000100; font.glyphs['^'].memory[5] = 0b00000000;
   font.glyphs[']'].memory[6] = 0b00000100; font.glyphs['^'].memory[6] = 0b00000000;
   font.glyphs[']'].memory[7] = 0b00000100; font.glyphs['^'].memory[7] = 0b00000000;
   font.glyphs[']'].memory[8] = 0b00111100; font.glyphs['^'].memory[8] = 0b00000000;
   font.glyphs[']'].memory[9] = 0b00000000; font.glyphs['^'].memory[9] = 0b00000000;

   font.glyphs['_'].memory[0] = 0b00000000; font.glyphs['`'].memory[0] = 0b00110000;
   font.glyphs['_'].memory[1] = 0b00000000; font.glyphs['`'].memory[1] = 0b00001100;
   font.glyphs['_'].memory[2] = 0b00000000; font.glyphs['`'].memory[2] = 0b00000000;
   font.glyphs['_'].memory[3] = 0b00000000; font.glyphs['`'].memory[3] = 0b00000000;
   font.glyphs['_'].memory[4] = 0b00000000; font.glyphs['`'].memory[4] = 0b00000000;
   font.glyphs['_'].memory[5] = 0b00000000; font.glyphs['`'].memory[5] = 0b00000000;
   font.glyphs['_'].memory[6] = 0b00000000; font.glyphs['`'].memory[6] = 0b00000000;
   font.glyphs['_'].memory[7] = 0b00000000; font.glyphs['`'].memory[7] = 0b00000000;
   font.glyphs['_'].memory[8] = 0b01111110; font.glyphs['`'].memory[8] = 0b00000000;
   font.glyphs['_'].memory[9] = 0b00000000; font.glyphs['`'].memory[9] = 0b00000000;

   font.glyphs['{'].memory[0] = 0b00001100; font.glyphs['|'].memory[0] = 0b00010000;
   font.glyphs['{'].memory[1] = 0b00010000; font.glyphs['|'].memory[1] = 0b00010000;
   font.glyphs['{'].memory[2] = 0b00010000; font.glyphs['|'].memory[2] = 0b00010000;
   font.glyphs['{'].memory[3] = 0b00010000; font.glyphs['|'].memory[3] = 0b00010000;
   font.glyphs['{'].memory[4] = 0b01100000; font.glyphs['|'].memory[4] = 0b00010000;
   font.glyphs['{'].memory[5] = 0b00010000; font.glyphs['|'].memory[5] = 0b00010000;
   font.glyphs['{'].memory[6] = 0b00010000; font.glyphs['|'].memory[6] = 0b00010000;
   font.glyphs['{'].memory[7] = 0b00010000; font.glyphs['|'].memory[7] = 0b00010000;
   font.glyphs['{'].memory[8] = 0b00001100; font.glyphs['|'].memory[8] = 0b00010000;
   font.glyphs['{'].memory[9] = 0b00000000; font.glyphs['|'].memory[9] = 0b00000000;

   font.glyphs['}'].memory[0] = 0b01100000; font.glyphs['~'].memory[0] = 0b00000000;
   font.glyphs['}'].memory[1] = 0b00010000; font.glyphs['~'].memory[1] = 0b00000000;
   font.glyphs['}'].memory[2] = 0b00010000; font.glyphs['~'].memory[2] = 0b00000000;
   font.glyphs['}'].memory[3] = 0b00010000; font.glyphs['~'].memory[3] = 0b01100010;
   font.glyphs['}'].memory[4] = 0b00001100; font.glyphs['~'].memory[4] = 0b10010010;
   font.glyphs['}'].memory[5] = 0b00010000; font.glyphs['~'].memory[5] = 0b10001100;
   font.glyphs['}'].memory[6] = 0b00010000; font.glyphs['~'].memory[6] = 0b00000000;
   font.glyphs['}'].memory[7] = 0b00010000; font.glyphs['~'].memory[7] = 0b00000000;
   font.glyphs['}'].memory[8] = 0b01100000; font.glyphs['~'].memory[8] = 0b00000000;
   font.glyphs['}'].memory[9] = 0b00000000; font.glyphs['~'].memory[9] = 0b00000000;

   /////////////////////////////////////////////////////////////////////////////////

   font.glyphs['0'].memory[0] = 0b00111100; font.glyphs['1'].memory[0] = 0b00001000;
   font.glyphs['0'].memory[1] = 0b01000010; font.glyphs['1'].memory[1] = 0b00011000;
   font.glyphs['0'].memory[2] = 0b01000010; font.glyphs['1'].memory[2] = 0b00101000;
   font.glyphs['0'].memory[3] = 0b01000110; font.glyphs['1'].memory[3] = 0b00001000;
   font.glyphs['0'].memory[4] = 0b01011010; font.glyphs['1'].memory[4] = 0b00001000;
   font.glyphs['0'].memory[5] = 0b01100010; font.glyphs['1'].memory[5] = 0b00001000;
   font.glyphs['0'].memory[6] = 0b01000010; font.glyphs['1'].memory[6] = 0b00001000;
   font.glyphs['0'].memory[7] = 0b01000010; font.glyphs['1'].memory[7] = 0b00001000;
   font.glyphs['0'].memory[8] = 0b00111100; font.glyphs['1'].memory[8] = 0b00001000;
   font.glyphs['0'].memory[9] = 0b00000000; font.glyphs['1'].memory[9] = 0b00000000;

   font.glyphs['2'].memory[0] = 0b00111100; font.glyphs['3'].memory[0] = 0b00111100;
   font.glyphs['2'].memory[1] = 0b01000010; font.glyphs['3'].memory[1] = 0b01000010;
   font.glyphs['2'].memory[2] = 0b01000010; font.glyphs['3'].memory[2] = 0b00000010;
   font.glyphs['2'].memory[3] = 0b00000100; font.glyphs['3'].memory[3] = 0b00000000;
   font.glyphs['2'].memory[4] = 0b00001000; font.glyphs['3'].memory[4] = 0b00011100;
   font.glyphs['2'].memory[5] = 0b00010000; font.glyphs['3'].memory[5] = 0b00000010;
   font.glyphs['2'].memory[6] = 0b00100000; font.glyphs['3'].memory[6] = 0b00000010;
   font.glyphs['2'].memory[7] = 0b01000000; font.glyphs['3'].memory[7] = 0b01000010;
   font.glyphs['2'].memory[8] = 0b01111110; font.glyphs['3'].memory[8] = 0b00111100;
   font.glyphs['2'].memory[9] = 0b00000000; font.glyphs['3'].memory[9] = 0b00000000;

   font.glyphs['4'].memory[0] = 0b00001000; font.glyphs['5'].memory[0] = 0b01111110;
   font.glyphs['4'].memory[1] = 0b00010000; font.glyphs['5'].memory[1] = 0b01000000;
   font.glyphs['4'].memory[2] = 0b00010000; font.glyphs['5'].memory[2] = 0b01000000;
   font.glyphs['4'].memory[3] = 0b00100100; font.glyphs['5'].memory[3] = 0b01111100;
   font.glyphs['4'].memory[4] = 0b00100100; font.glyphs['5'].memory[4] = 0b01000010;
   font.glyphs['4'].memory[5] = 0b01111110; font.glyphs['5'].memory[5] = 0b00000010;
   font.glyphs['4'].memory[6] = 0b00000100; font.glyphs['5'].memory[6] = 0b00000010;
   font.glyphs['4'].memory[7] = 0b00000100; font.glyphs['5'].memory[7] = 0b01000010;
   font.glyphs['4'].memory[8] = 0b00000100; font.glyphs['5'].memory[8] = 0b00111100;
   font.glyphs['4'].memory[9] = 0b00000000; font.glyphs['5'].memory[9] = 0b00000000;

   font.glyphs['6'].memory[0] = 0b00111100; font.glyphs['7'].memory[0] = 0b01111110;
   font.glyphs['6'].memory[1] = 0b01000010; font.glyphs['7'].memory[1] = 0b00000010;
   font.glyphs['6'].memory[2] = 0b01000000; font.glyphs['7'].memory[2] = 0b00000100;
   font.glyphs['6'].memory[3] = 0b01111100; font.glyphs['7'].memory[3] = 0b00000100;
   font.glyphs['6'].memory[4] = 0b01000010; font.glyphs['7'].memory[4] = 0b00111110;
   font.glyphs['6'].memory[5] = 0b01000010; font.glyphs['7'].memory[5] = 0b00001000;
   font.glyphs['6'].memory[6] = 0b01000010; font.glyphs['7'].memory[6] = 0b00010000;
   font.glyphs['6'].memory[7] = 0b01000010; font.glyphs['7'].memory[7] = 0b00010000;
   font.glyphs['6'].memory[8] = 0b00111100; font.glyphs['7'].memory[8] = 0b00100000;
   font.glyphs['6'].memory[9] = 0b00000000; font.glyphs['7'].memory[9] = 0b00000000;

   font.glyphs['8'].memory[0] = 0b00111100; font.glyphs['9'].memory[0] = 0b00111100;
   font.glyphs['8'].memory[1] = 0b01000010; font.glyphs['9'].memory[1] = 0b01000010;
   font.glyphs['8'].memory[2] = 0b01000010; font.glyphs['9'].memory[2] = 0b01000010;
   font.glyphs['8'].memory[3] = 0b01000010; font.glyphs['9'].memory[3] = 0b01000010;
   font.glyphs['8'].memory[4] = 0b00111100; font.glyphs['9'].memory[4] = 0b00111110;
   font.glyphs['8'].memory[5] = 0b01000010; font.glyphs['9'].memory[5] = 0b00000010;
   font.glyphs['8'].memory[6] = 0b01000010; font.glyphs['9'].memory[6] = 0b00000010;
   font.glyphs['8'].memory[7] = 0b01000010; font.glyphs['9'].memory[7] = 0b00000100;
   font.glyphs['8'].memory[8] = 0b00111100; font.glyphs['9'].memory[8] = 0b00111000;
   font.glyphs['8'].memory[9] = 0b00000000; font.glyphs['9'].memory[9] = 0b00000000;
}

function void draw_text(exo_texture *backbuffer, s32 x, s32 y, rectangle bounds, char *text)
{
   s32 bounded_minx = MAXIMUM(0, bounds.x);
   s32 bounded_miny = MAXIMUM(0, bounds.y);

   s32 bounded_maxx = MINIMUM(bounds.x + bounds.width, backbuffer->width);
   s32 bounded_maxy = MINIMUM(bounds.y + bounds.height, backbuffer->height);

   for(u32 index = 0; index < strlen(text); ++index)
   {
      s32 minx = MAXIMUM(x, bounded_minx);
      s32 miny = MAXIMUM(y, bounded_miny);

      s32 maxx = MINIMUM(x + (FONT_WIDTH * FONT_SCALE),  bounded_maxx);
      s32 maxy = MINIMUM(y + (FONT_HEIGHT * FONT_SCALE), bounded_maxy);

      char character = text[index];
      bitmap_glyph glyph = font.glyphs[character];

      for(s32 destinationy = miny; destinationy < maxy; destinationy += FONT_SCALE)
      {
         u8 charactery = (u8)(destinationy - y) / FONT_SCALE;
         u8 row = glyph.memory[charactery];

         for(s32 destinationx = minx; destinationx < maxx; destinationx += FONT_SCALE)
         {
            u8 characterx = (u8)(destinationx - x) / FONT_SCALE;
            u8 offset = FONT_WIDTH - characterx - 1;

            if((row >> offset) & 0x1)
            {
               backbuffer->memory[((destinationy + 0) * backbuffer->width) + destinationx + 0] = 0xFF000000;
#if FONT_SCALE == 2
               backbuffer->memory[((destinationy + 0) * backbuffer->width) + destinationx + 1] = 0xFF000000;
               backbuffer->memory[((destinationy + 1) * backbuffer->width) + destinationx + 0] = 0xFF000000;
               backbuffer->memory[((destinationy + 1) * backbuffer->width) + destinationx + 1] = 0xFF000000;
#endif
            }
         }
      }

      x += (FONT_WIDTH * FONT_SCALE);
   }
}
