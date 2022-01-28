#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define word uint16_t

uint8_t VGA_8158_GAMEPAL[769];

void fskip(FILE *fp, int num_bytes)
{
   int i;
   for (i=0; i<num_bytes; i++)
      fgetc(fp);
}


static void Load_PAL(const char *file)
{
	FILE *fp;
	int32_t index;
	int width, height, num_colors;

	/* open the file */
	if ((fp = fopen(file,"rb")) == NULL)
	{
		printf("Error opening file %s.\n", file);
		exit(1);
	}

  /* check to see if it is a valid bitmap file */
	if (fgetc(fp)!='B' || fgetc(fp)!='M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n",file);
		exit(1);
	}
  
	/* read in the width and height of the image, and the
	number of colors used; ignore the rest */
	fskip(fp,16);
	fread(&width, sizeof(word), 1, fp);
	fskip(fp,2);
	fread(&height,sizeof(word), 1, fp);
	fskip(fp,22);
	fread(&num_colors,sizeof(word), 1, fp);
	fskip(fp,6);
  
	/* assume we are working with an 8-bit file */
	if (num_colors==0) num_colors=256;

	for(index=0;index<num_colors;index++)
	{
		/*
		That was used for DOS VGA palette
		VGA_8158_GAMEPAL[(int)(index*3+2)] = fgetc(fp) >> 2;
		VGA_8158_GAMEPAL[(int)(index*3+1)] = fgetc(fp) >> 2;
		VGA_8158_GAMEPAL[(int)(index*3+0)] = fgetc(fp) >> 2;
		*/
		VGA_8158_GAMEPAL[(int)(index*3+2)] = fgetc(fp);
		VGA_8158_GAMEPAL[(int)(index*3+1)] = fgetc(fp);
		VGA_8158_GAMEPAL[(int)(index*3+0)] = fgetc(fp);
		fgetc(fp);
	}	

	fclose(fp);
}

uint16_t RGB2PAL(int r, int g, int b, FILE* f)
{
	uint16_t ui___P;
	float fl___R = (float) r; // 8-bit (0-255)
	float fl___G = (float) g; // 8-bit (0-255)
	float fl___B = (float) b; // 8-bit (0-255)

	float fl___Y = ( 0.2990f * fl___R) + (0.5870f * fl___G) + (0.1140f * fl___B);
	float fl___U = (-0.1686f * fl___R) - (0.3311f * fl___G) + (0.4997f * fl___B) + 128.0f;
	float fl___V = ( 0.4998f * fl___R) - (0.4185f * fl___G) - (0.0813f * fl___B) + 128.0f;

	if (fl___Y <   0.0f) fl___Y =   0.0f;
	if (fl___Y > 255.0f) fl___Y = 255.0f;
	if (fl___U <   0.0f) fl___U =   0.0f;
	if (fl___U > 255.0f) fl___U = 255.0f;
	if (fl___V <   0.0f) fl___V =   0.0f;
	if (fl___V > 255.0f) fl___V = 255.0f;
	
	//fprintf("R %d, G %d, B%d\n", r, g, b);

	unsigned ui___Y = (unsigned) fl___Y;
	unsigned ui___U = (unsigned) fl___U;
	unsigned ui___V = (unsigned) fl___V;

	ui___U = (ui___U + 8 ) >> 4;
	ui___V = (ui___V + 8 ) >> 4;
	ui___P = (ui___Y << 8 ) + (ui___U << 4) + ui___V;	
	
	fprintf(f, "0x%x, \n", ui___P);
        
	return (uint16_t) ui___P;
}


int main(int argc, char* argv[])
{
	int index;
	FILE* fp;
	if (argc < 3)
	{
		printf("BMP2YUV\nTool for converting palettes from BMP files to a YUV palette suitable for PC-FX\nbmp2yuv mybmp.bmp yuv.txt\n");
		return 0;
	}
	
	Load_PAL(argv[1]);
	
	fp = fopen(argv[2], "w");
	for(index=0;index<256;index++)
	{
		RGB2PAL(VGA_8158_GAMEPAL[(index*3)+0], VGA_8158_GAMEPAL[(index*3)+1], VGA_8158_GAMEPAL[(index*3)+2], fp);
	}
	fclose(fp);
	
}
