#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <Windows.h>
#include <cstdlib>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds


extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define MAX_JUMP_HEIGHT 200


// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void skacz(int *IsJumping, int *IsFalling,  int *horse_position_y, unsigned int *jump )
{
	if (*IsJumping == 1) { //printf("SKACZEEEE");

		if (*IsFalling == 1 && *jump > 0) { //spadek
			(*horse_position_y)++;
			(*jump)--;

			if (*jump == 0) { //koniec skoku
				*IsJumping = 0;
				*IsFalling = 0;
			}
		}

		else { //wznoszenie sie
			(*jump)++;
			(*horse_position_y)--;

			if (*jump == MAX_JUMP_HEIGHT / 2)
				*IsFalling = 1;
		}

	}

	if (*IsJumping == 2) { //printf("DRUGI SKACZEEEE");

		if (*IsFalling == 2 && *jump > 0) { //spadek
			(*horse_position_y)++;
			(*jump)--;

			if (*jump == 0) { //koniec skoku
				*IsJumping = 0;
				*IsFalling = 0;
			}
		}

		else { //wznoszenie sie
			(*jump)++;
			(*horse_position_y)--;

			if (*jump == MAX_JUMP_HEIGHT)
				*IsJumping = 2;
		}
	}
}



// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, begin, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface* screen, * charset, * click_to_play, * tlo_tecza;
	SDL_Surface* eti;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

	// okno konsoli nie jest widoczne, je¿eli chcemy zobaczyæ
	// komunikaty wypisywane printf-em trzeba w opcjach:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// zmieniæ na "Console"
	// console window is not visible, to see the printf output
	// the option:
	// project -> szablon2 properties -> Linker -> System -> Subsystem
	// must be changed to "Console"
	printf("wyjscie printfa trafia do tego okienka\n");
	printf("printf output goes here\n");

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2017");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	eti = SDL_LoadBMP("./eti.bmp");
	if (eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	click_to_play = SDL_LoadBMP("./click_to_play.bmp");
	if (click_to_play == NULL) {
		printf("SDL_LoadBMP(click_to_play.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};


	tlo_tecza = SDL_LoadBMP("./tlo_tecza.bmp");
	if (tlo_tecza == NULL) {
		printf("SDL_LoadBMP(tlo_tecza.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};






	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	worldTime = 0;
	distance = 0;
	etiSpeed = 1;

	begin = 0;
	quit = 0;
#ifdef TEST
	while (begin != 1) {
		SDL_FillRect(screen, NULL, czarny);
		DrawSurface(screen, click_to_play, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Wcisnij n zeby zaczac");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//	SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		//	//SDL_FillRect(screen, &screen->clip_rect, color);
		//	//SDL_BlitSurface(low, &rects[animation_frame], screen, &rect);
		//	//SDL_FillRect(screen, NULL, czarny);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					quit = 1;
					begin = 1;
				}
				else if (event.key.keysym.sym == SDLK_n) begin = 1;
				break;
			case SDL_QUIT:
				quit = 1;
				begin = 1;
				break;
			};
		};
	}
#endif
	int rendering_map = 0;
	SDL_Surface* map;
	while (rendering_map != 1){
		map = SDL_LoadBMP("./map7.bmp");
		if (map == NULL) {
			printf("SDL_LoadBMP(map7.bmp) error: %s\n", SDL_GetError());
			SDL_FreeSurface(charset);
			SDL_FreeSurface(screen);
			SDL_DestroyTexture(scrtex);
			SDL_DestroyWindow(window);
			SDL_DestroyRenderer(renderer);
			SDL_Quit();
			return 1;
		};
		SDL_SetColorKey(map, true, czarny);

		rendering_map = 1;
	}





		int surface_height = 300;

	int horse_position_x = 45;
	int horse_position_y = surface_height - 45;

	int number_of_collisions = 0;

	int controlling_keys = 0;
	int mnoznik_przyspieszenia = 1;
	int IsJumping = 0, IsFalling = 0;
	unsigned int jump = 0;
	int wysokosc_nad_ziemia = 0;
	int dystans = 0;
	int spadek = 0;


	int przeszkoda[3][10] = { 0 };
	for(int k=1; k<3; k++)
	for (int i = 0; i < 10; i++)
		przeszkoda[k][i] = k*700 + i;



	int dziura[100] = { 0 };
	for (int i = 0; i < 100; i++)
		dziura[i] = 1000 + i;

	int powtorzenie = 2;
	while (!quit) {
		t2 = SDL_GetTicks();

		// w tym momencie t2-t1 to czas w milisekundach,
		// jaki uplyna³ od ostatniego narysowania ekranu
		// delta to ten sam czas w sekundach
		// here t2-t1 is the time in milliseconds since
		// the last screen was drawn
		// delta is the same time in seconds
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;

		distance += etiSpeed * delta;

		fpsTimer += delta;
		//if (fpsTimer > 0.5) {
		//		fps = frames * 2;
		//		frames = 0;
		//		fpsTimer -= 0.5;
		//	};

		SDL_FillRect(screen, NULL, czarny);

		float mapx = -frames + 3200 / 2;

		//tlo
		DrawSurface(screen, tlo_tecza, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);




		//jednorozec
		DrawSurface(screen, eti, horse_position_x, horse_position_y);

#ifdef TEST
		//przeszkody
			//printf("%d    %4.2f    %d   \n", dystans+90, mapx, number_of_collisions);
		for (int k = 1; k < 3; k++)
			for (int i = 0; i < 10; i++)				
			if (dystans + 90 == przeszkoda[k][i] && horse_position_y > 250 && horse_position_y < surface_height)
			{
				number_of_collisions++;
				printf("Kolizjaaaaa nr %d ", number_of_collisions);
			}

			
			
		//dziury w podlozu

		for (int i = 0; i < 100; i++)
			if (dystans == dziura[i] && IsJumping == 0)
			{
				spadek = 1;
			}
		printf("%d     %d   \n", horse_position_y, dystans); //info

		if (spadek == 1)
		{
			horse_position_y++;
		}
		if (horse_position_y == surface_height - 44 && spadek == 0) horse_position_y = surface_height - 45; //blokowanie przez podloze
#endif




		//mapa
		DrawSurface(screen, map, mapx, surface_height);

		//kolejna czesc mapy
		if (mapx < -960 && mapx > -1600) {
			int diff = 3200 + mapx;
			DrawSurface(screen, map, diff, surface_height);

		}

		//printf("%4.2f \n", mapx); //info
	

		// tekst informacyjny / info text
		DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
		sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		sprintf(text, "Esc - wyjscie, number_of_collisions= %d", number_of_collisions);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);



		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
				else if (event.key.keysym.sym == SDLK_d){
					controlling_keys++;
					if (controlling_keys == 2) controlling_keys = 0;
				}
				else if (controlling_keys == 0) {
					if (event.key.keysym.sym == SDLK_UP){//skok
							 if (IsJumping == 0) IsJumping = 1;
						else if (IsJumping == 1 && IsFalling == 1) IsJumping = 2;
						
							 
					}
					else if (event.key.keysym.sym == SDLK_DOWN) horse_position_y += 1; //opadniecie
					else if (event.key.keysym.sym == SDLK_RIGHT)frames += 50; //zryw
				}
				else if (controlling_keys == 1) {
						 if (event.key.keysym.sym == SDLK_z)	horse_position_y -= 30;//skok
					else if (event.key.keysym.sym == SDLK_DOWN) horse_position_y += 1;//opadniecie
					else if (event.key.keysym.sym == SDLK_x)	frames += 50; //zryw

				}
				break;
			case SDL_KEYUP:
			{
				IsFalling++;
				if (IsFalling == 3)
					IsFalling = 2;
				if (IsFalling == 1 && jump == 0)
					IsFalling = 0;
			}
				
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			};
		};


		if (horse_position_y == surface_height-44) horse_position_y = surface_height - 45; //blokowanie przez podloze

		for(int i=0; i<10; i++)
		if (horse_position_x ==  mapx-200*i && horse_position_y>250 && horse_position_y < surface_height)
		{	
			sprintf(text, "Kolizja");
			DrawString(screen, horse_position_x, surface_height - 45 +50, text, charset);
			number_of_collisions++;
			printf("Kolizjaaaaaaaaaaaaaaaaa nr %d \n" , number_of_collisions);

		}

		wysokosc_nad_ziemia = surface_height- horse_position_y - 45;
		printf("jump: %d IsJumping: %d IsFalling: %d horse_position_y: %d \n", jump, IsJumping, IsFalling, horse_position_y); //info

		if (IsJumping != 0) skacz(&IsJumping, &IsFalling, &horse_position_y, &jump);

		




		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		//		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		
		frames += mnoznik_przyspieszenia;

		if (frames > 3200) {
			frames = 0;
			//mnoznik_przyspieszenia += 1;
		}
	};





	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};
