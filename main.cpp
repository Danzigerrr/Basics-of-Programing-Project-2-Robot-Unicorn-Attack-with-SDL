#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SZEROKOSC_MAPY_BMP 3200
#define SURFACE_HEIGHT 300
#define MAX_JUMP_HEIGHT 200
#define SZEROKOSC_KONIA 90
#define WYSOKOSC_KONIA 90
#define ILOSC_PRZESZKOD 2
#define SZEROKOSC_PRZESZKODY 10
#define ODL_POM_PRZESZKODAMI 700
#define ILOSC_WROZEK 2
#define ODL_POM_WROZKAMI 800
#define ILOSC_STALAKTYTOW 2
#define ODL_POM_STALAKTYTAMI 1000
#define ILOSC_DZIUR 2
#define SZEROKOSC_DZIURY 200
#define ODL_POM_DZIURAMI 1000
#define ILOSC_DOD_PODLOZA 1
#define SZEROKOSC_DOD_PODLOZA 200
#define WYSOKOSC_DODATKOWEGO_PODLOZA 60
#define ODL_POM_DOD_PODLOZAMI 700


#ifndef funkcje


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


void zwolnienie_powierzchni(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

}

void update_the_screen(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer){
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	//		SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void zmierz_czas(int *t1,  double *delta, double *worldTime ){
	int t2 = SDL_GetTicks();
	*delta = (t2 - *t1) * 0.001;
	*t1 = t2;
	*worldTime += *delta;

}


#endif 

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, begin, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance, etiSpeed;
	SDL_Event event;
	SDL_Surface *screen, *charset, *click_to_play, *konik, *tlo_tecza, *map, *ikonka_zycia, *wrozki, *znaczek_eti, *star, * stalaktyt, *dziuraa, *grass, * ziemniaki;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;

#ifndef systemowe
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
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Robot Unicorn Attack");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
#endif

#ifndef Ładowanie_obrazkow

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
	};
	SDL_SetColorKey(charset, true, 0x000000);

	konik = SDL_LoadBMP("./konik.bmp");
	if (konik == NULL) {
		printf("SDL_LoadBMP(konik.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(konik, true, czarny);

	click_to_play = SDL_LoadBMP("./click_to_play.bmp");
	if (click_to_play == NULL) {
		printf("SDL_LoadBMP(click_to_play.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

	tlo_tecza = SDL_LoadBMP("./tlo_tecza.bmp");
	if (tlo_tecza == NULL) {
		printf("SDL_LoadBMP(tlo_tecza.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

	map = SDL_LoadBMP("maps/map18.bmp");
	if (map == NULL) {
		printf("SDL_LoadBMP(map12.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(map, true, czarny);

	ikonka_zycia = SDL_LoadBMP("./ikonka_zycia.bmp");
	if (ikonka_zycia == NULL) {
		printf("SDL_LoadBMP(ikonka_zycia.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(ikonka_zycia, true, czarny);

	wrozki = SDL_LoadBMP("./wrozka1.bmp");
	if (wrozki == NULL) {
		printf("SDL_LoadBMP(wrozka1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(wrozki, true, czarny);

	znaczek_eti = SDL_LoadBMP("./eti.bmp");
	if (znaczek_eti == NULL) {
		printf("SDL_LoadBMP(znaczek_eti.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

	star = SDL_LoadBMP("./star1.bmp");
	if (star == NULL) {
		printf("SDL_LoadBMP(star1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(star, true, czarny);

	stalaktyt = SDL_LoadBMP("./stal2.bmp");
	if (stalaktyt == NULL) {
		printf("SDL_LoadBMP(stal1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};
	SDL_SetColorKey(stalaktyt, true, czarny);

	dziuraa = SDL_LoadBMP("./dziura1.bmp");
	if (dziuraa == NULL) {
		printf("SDL_LoadBMP(dziura1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

	grass = SDL_LoadBMP("./grass2.bmp");
	if (grass == NULL) {
		printf("SDL_LoadBMP(grass1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

	ziemniaki = SDL_LoadBMP("./dodatkowe1.bmp");
	if (ziemniaki == NULL) {
		printf("SDL_LoadBMP(grass1.bmp) error: %s\n", SDL_GetError());
		zwolnienie_powierzchni(screen, scrtex, window, renderer);
		return 1;
	};

#endif 

	begin = 0;
	quit = 0;

	bool control_with_arrows = true;

	while (!quit) {

		while (!begin) {
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



			DrawSurface(screen, click_to_play, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			

			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
			sprintf(text, "Wcisnij n zeby zaczac");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

			update_the_screen(screen, scrtex, renderer);


			//	//SDL_FillRect(screen, &screen->clip_rect, color);
			//	//SDL_BlitSurface(low, &rects[animation_frame], screen, &rect);
			//	//SDL_FillRect(screen, NULL, czarny);
		
		}

		//tworzenie nowej gry - inicjalizacja zmiennych
		t1 = SDL_GetTicks();
		frames = 0;
		fpsTimer = 0;
		fps = 0;
		worldTime = 0;
		distance = 0;
		etiSpeed = 1;

		float mnoznik_przyspieszenia = 1.0;
		int IsJumping = 0, IsFalling = 0;
		unsigned int jump = 0;
	
		int dystans = 0;

		int zryw = 5;
		int dlugosc_klatek_zrywu = 0;
		int ilosc_dodawanych_klatek_w_zrywie = mnoznik_przyspieszenia;

		int ilosc_zyc = 3;
		bool ask_if_play_again = false;

		int spadl_na_dodatkowe_podloze = 0;

		int max_girl_move= 0;
		int girl_moved = 0;
		
		int punkty = 0;
		int przeszkoda_zotała_rozbita[ILOSC_PRZESZKOD + 1] = { 0 }; //numerowane od 1
		int przeszkoda_ominieta[ILOSC_PRZESZKOD + 1] = { 0 };//numerowane od 1
		int kolejna_gwiazda_rozbita = 0;

		int wrozka_zostala_zebrana[ILOSC_WROZEK + 1] = { 0 }; //numerowane od 1
		int wrozka_ominieta[ILOSC_WROZEK + 1] = { 0 };//numerowane od 1
		int kolejna_wrozka_zebrana = 0;

		SDL_Rect horse;
		horse.x = SZEROKOSC_KONIA / 2 - 35;
		horse.y = SURFACE_HEIGHT - SZEROKOSC_KONIA / 2 - 25;
		horse.w = 75;
		horse.h = 75;

		SDL_Rect girl;
		girl.x = SCREEN_WIDTH;
		girl.y = 180 - 15;
		girl.w = 30;
		girl.h = 30;

		SDL_Rect gwiazda;
		gwiazda.x = SCREEN_WIDTH;
		gwiazda.y = 260;
		gwiazda.w = 50;
		gwiazda.h = 50;

		SDL_Rect stal;
		stal.x = SCREEN_WIDTH;
		stal.y = 40;
		stal.w = 55;
		stal.h = 60;

		SDL_Rect podloze;
		podloze.x = 0;
		podloze.y = SURFACE_HEIGHT + 30;
		podloze.w = 10;
		podloze.h = 160;

		SDL_Rect hole;
		hole.x = 0;
		hole.y = SURFACE_HEIGHT + 10;
		hole.w = SZEROKOSC_DZIURY;
		hole.h = 160;

		SDL_Rect dodatkowe;
		dodatkowe.x = 0;
		dodatkowe.y = SURFACE_HEIGHT- WYSOKOSC_DODATKOWEGO_PODLOZA+10;
		dodatkowe.w = SZEROKOSC_DOD_PODLOZA;
		dodatkowe.h = WYSOKOSC_DODATKOWEGO_PODLOZA;

		while (ilosc_zyc > 0) {
			if (!ask_if_play_again) {

				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_KEYDOWN:
						if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; //wyjscie z programu

						else if (event.key.keysym.sym == SDLK_d) { // zmiana ustawien sterowania
							if (control_with_arrows) control_with_arrows = false;
							else control_with_arrows = true;

						}

						else if (control_with_arrows) {
							if (event.key.keysym.sym == SDLK_UP) {//skok
								if (IsJumping == 0) IsJumping = 1;
								else if (IsJumping == 1 && IsFalling == 1) IsJumping = 2;
							}
							else if (event.key.keysym.sym == SDLK_RIGHT) //zryw
							{
								if (zryw == 5) {
									zryw = 2;

									if (jump > 0) {
										IsJumping = 1;
										IsFalling = 1;
									}
									if (jump == 0) {
										IsJumping = 0;
										IsFalling = 0;
									}

								}
							}
						}

						else if (!control_with_arrows) {
							if (event.key.keysym.sym == SDLK_z) { //skok
								if (IsJumping == 0) IsJumping = 1;
								else if (IsJumping == 1 && IsFalling == 1) IsJumping = 2;
							}

							else if (event.key.keysym.sym == SDLK_x) //zryw
							{
								if (zryw == 5) {
									zryw = 2;

									if (jump > 0) {
										IsJumping = 1;
										IsFalling = 1;
									}
									if (jump == 0) {
										IsJumping = 0;
										IsFalling = 0;
									}

								}
							}
						}
						break;

					case SDL_KEYUP:
						if (control_with_arrows) {
							if (event.key.keysym.sym == SDLK_UP) { //spadanie po skoku
								IsFalling++;
								if (IsFalling == 3)
									IsFalling = 2;
								if (IsFalling == 1 && jump == 0)
									IsFalling = 0;
							}
							else if (event.key.keysym.sym == SDLK_RIGHT) {
								zryw = 3; //zryw

							}
						}

						if (!control_with_arrows) {
							if (event.key.keysym.sym == SDLK_z) { //spadanie po skoku
								IsFalling++;
								if (IsFalling == 3)
									IsFalling = 2;
								if (IsFalling == 1 && jump == 0)
									IsFalling = 0;
							}
							else if (event.key.keysym.sym == SDLK_x) {
								zryw = 3; //zryw
							}
						}
						break;
					case SDL_QUIT:
						quit = 1;
						break;
					};
				};

				zmierz_czas(&t1, &delta, &worldTime);

				
#ifndef tła

					//tlo
					DrawSurface(screen, tlo_tecza, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

					//ilosc_zyc
					for (int i = 0; i < ilosc_zyc; i++)
						DrawSurface(screen, ikonka_zycia, 40 + i * 60, SCREEN_HEIGHT - 50);

					//jednorozec
					SDL_BlitSurface(konik, NULL, screen, &horse);

					float mapx = -frames + SZEROKOSC_MAPY_BMP / 2;
					//mapa i stalaktyty
					DrawSurface(screen, map, mapx, SURFACE_HEIGHT);

					//pokazuje kolejna czesc mapy
					if (mapx < SCREEN_WIDTH - SZEROKOSC_MAPY_BMP / 2 && mapx > -SZEROKOSC_MAPY_BMP / 2) {
						int diff = SZEROKOSC_MAPY_BMP + mapx;
						DrawSurface(screen, map, diff, SURFACE_HEIGHT);

					}
#endif
					//przy klikaniu n poprawnie ladowac nowa gre - stalkatyty itp

					//losowe odleglosci ==> bazowa odleglosc + random 0 do 200


				if (zryw != 0){
							//zryw = 2 kon jest podczas zrywu --> zryw aktywny
							//zryw = 3 powracanie na pozycje
							//zryw = 4 zryw sie skonczyl - mozliwy dodatkowy skok
							//zryw = 5 kon upadl na ziemie po zakonczeniu zrywu lub dodtakowego skoku, pelna gotowosc do kolejnego zrywu

							if (zryw == 2)
							{
								if (dlugosc_klatek_zrywu < 100 * mnoznik_przyspieszenia)
								{
									dlugosc_klatek_zrywu += 2 * ilosc_dodawanych_klatek_w_zrywie;
									dystans += 2 * ilosc_dodawanych_klatek_w_zrywie;
									horse.x += 2 * ilosc_dodawanych_klatek_w_zrywie;
								}
								else {
									zryw = 3;
								}
							}
							if (zryw == 3) {
								if (dlugosc_klatek_zrywu > 0)
								{
									dlugosc_klatek_zrywu -= ilosc_dodawanych_klatek_w_zrywie;
									dystans -= ilosc_dodawanych_klatek_w_zrywie;
									horse.x -= ilosc_dodawanych_klatek_w_zrywie;

								}
								else zryw = 4;
							}
							if (zryw == 4 || zryw == 5) {


								if (IsJumping != 0)  //skacz(&IsJumping, &IsFalling, &horse_position_y, &jump);
								{
									if (IsJumping == 1) { //printf("SKACZEEEE");

										if (IsFalling == 1 && jump > 0) { //spadek
											
											(jump)--;
											horse.y++;

											if (jump == 0) { //koniec skoku
												IsJumping = 0;
												IsFalling = 0;
												zryw = 5;
												spadl_na_dodatkowe_podloze = 0;
											}
										}

										else { //wznoszenie sie
											(jump)++;
											horse.y--;

											if (jump == MAX_JUMP_HEIGHT / 2)
												IsFalling = 1;
										}

									}

									if (IsJumping == 2) { //printf("DRUGI SKACZEEEE");

										if (IsFalling == 2 && jump > 0) { //spadek
											(jump)--;
											horse.y++;

											if (jump == 0) { //koniec skoku
												IsJumping = 0;
												IsFalling = 0;
												zryw = 5;
												spadl_na_dodatkowe_podloze = 0;
											}
										}

										else { //wznoszenie sie
											(jump)++;
											horse.y--;

											if (jump == MAX_JUMP_HEIGHT)
												IsFalling = 2;
										}
									}
								}
								else zryw = 5;
							}
							//zablokowanie skoku wychodzacego po za gre
							if (horse.y == 0) {
								IsJumping = 2;
								IsFalling = 2;
							}
						}

#ifdef TEST
				//dodatkowe podloze
				for (int k = 1; k <= ILOSC_DOD_PODLOZA; k++) {

					//spadl_na_dodatkowe_podloze
					// 0 jest przed obszarem dod podloza
					// 1 aktualnie przebywa na dod podlozu
					// 2 zaczal spadac z dodatkowego podloza

					int start_showing = k * ODL_POM_DOD_PODLOZAMI;
					int end_showing = k * ODL_POM_DOD_PODLOZAMI + SCREEN_WIDTH + SZEROKOSC_DOD_PODLOZA;


					if (start_showing < frames && frames < end_showing) {

						dodatkowe.x = start_showing + SCREEN_WIDTH - frames;

						SDL_BlitSurface(ziemniaki, NULL, screen, &dodatkowe);

						if (spadl_na_dodatkowe_podloze == 0) {
							if (SDL_HasIntersection(&horse, &dodatkowe) && jump < WYSOKOSC_DODATKOWEGO_PODLOZA) {
								printf("zderzenie czolowe \n");
								ask_if_play_again = true;
								ilosc_zyc--;
							}

							if (SDL_HasIntersection(&horse, &dodatkowe) && jump > WYSOKOSC_DODATKOWEGO_PODLOZA) {
								jump = 0;
								IsFalling = 0;
								IsJumping = 0;
								spadl_na_dodatkowe_podloze = 1;
							}
						}
					}
					if (frames == end_showing) {
						jump += WYSOKOSC_DODATKOWEGO_PODLOZA;
						spadl_na_dodatkowe_podloze = 2;
					}
				}
				if (spadl_na_dodatkowe_podloze == 2 && jump > 0 && IsJumping == 0) {
					if (jump == 0)
						spadl_na_dodatkowe_podloze = 0;
					else {
						jump--;
						horse.y++;
					}
				}

				//dziury w podlozu
				for (int k = 1; k <= ILOSC_DZIUR; k++) {
					int start_showing = k * ODL_POM_DZIURAMI;
					int end_showing = k * ODL_POM_DZIURAMI + SCREEN_WIDTH + hole.w;


					if (start_showing < frames && frames < end_showing) {


						if (SDL_HasIntersection(&horse, &hole) && IsJumping == 0) {
							printf("DZIURA \n");
							horse.y++;
							jump--;
						}

						hole.x = start_showing + SCREEN_WIDTH - frames;

						SDL_BlitSurface(dziuraa, NULL, screen, &hole);

						//DrawRectangle(screen, hole.x, hole.y, hole.w, hole.h, niebieski, czerwony);
					}

					// zderzenie z poczatkiem terenu za dziura
					if (end_showing < frames && frames < end_showing + SZEROKOSC_DZIURY) {
						podloze.x = start_showing + SCREEN_WIDTH - frames + SZEROKOSC_DZIURY;
						//DrawRectangle(screen, podloze.x, podloze.y, podloze.w, podloze.h, niebieski, zielony);

						if (SDL_HasIntersection(&horse, &podloze)) {
							ask_if_play_again = true;
							ilosc_zyc--;
						}
					}

					//spadniecie na dolny koniec ekranu
					if (jump == -160) {
						ask_if_play_again = true;
						ilosc_zyc--;
					}

				}
#endif
				//przeszkody
				for (int k = 1; k <= ILOSC_PRZESZKOD; k++) {
					int start_showing = k * ODL_POM_PRZESZKODAMI;
					int end_showing = k * ODL_POM_PRZESZKODAMI + SCREEN_WIDTH + gwiazda.w;

					if (frames == end_showing && przeszkoda_zotała_rozbita[k] == 0)
						kolejna_gwiazda_rozbita = 0;

					if (frames == 0 || frames == end_showing) { // odsuniecie wrozki na bok i przugtowanie jej na kolejne pokazanie
						gwiazda.x = SCREEN_WIDTH;
						gwiazda.y = 260;
					}

					if (start_showing < frames && frames < end_showing) {

						//odleglosc = sqrt(pow(wrozkaX - horse_position_x, 2) + pow(wrozkaY - horse_position_y, 2) + konikR + wrozkaR);
						if (przeszkoda_zotała_rozbita[k] == 0)
						{
							if (SDL_HasIntersection(&horse, &gwiazda)) {
								if (zryw == 2) {
									przeszkoda_zotała_rozbita[k] = 1;
									kolejna_gwiazda_rozbita++;
									punkty += 100 * kolejna_gwiazda_rozbita;
									printf("rozbita \n");
								}
								else {
									//printf("Kolizjaaaaaaaaaaaa");
									ask_if_play_again = true;
									ilosc_zyc--;
								}
							}

							gwiazda.x = start_showing + SCREEN_WIDTH - frames;

							SDL_BlitSurface(star, NULL, screen, &gwiazda);

						}
						if (end_showing - frames <= 2 * mnoznik_przyspieszenia && przeszkoda_zotała_rozbita[k] == 0) {
							kolejna_gwiazda_rozbita = 0;
							printf("ominieta gwiazda\n");
						}

					}

				}

#ifdef TEST
				//wrozki
				for (int k = 1; k <= ILOSC_WROZEK; k++) {
					int start_showing = k * ODL_POM_WROZKAMI;
					int end_showing = k * ODL_POM_WROZKAMI + SCREEN_WIDTH + girl.w;

					if (start_showing < frames && frames < end_showing) {

						//odleglosc = sqrt(pow(wrozkaX - horse_position_x, 2) + pow(wrozkaY - horse_position_y, 2) + konikR + wrozkaR);
						if (wrozka_zostala_zebrana[k] == 0)
						{
							if (SDL_HasIntersection(&horse, &girl) && wrozka_zostala_zebrana[k] != 1) { // zebranie wrozki
								wrozka_zostala_zebrana[k] = 1;
								kolejna_wrozka_zebrana++;
								punkty += 10 * kolejna_wrozka_zebrana;
								printf("zebranaaaaaa\n");
							}

							if (girl_moved < max_girl_move) {
								girl_moved++;
								if (girl_moved % 2) {
									if (girl_moved < max_girl_move / 2)  girl.y--;
									else girl.y++;
								}
							}

							else {
								girl_moved = 0;
								srand((unsigned)time(0));
								max_girl_move = 100 + (rand() % 100);
							}


							if (end_showing - frames <= 2*mnoznik_przyspieszenia && wrozka_zostala_zebrana[k] == 0 ){
								kolejna_wrozka_zebrana = 0;
								printf("ominieta\n");
							}

							girl.x = start_showing + SCREEN_WIDTH - frames;

							if (girl.y <= 50) girl_moved = max_girl_move / 2; // ograniczenie pionowych ruchow wrozki
							else if (girl.y >= 300) girl_moved = 0;

							SDL_BlitSurface(wrozki, NULL, screen, &girl);

						}
					}
				}

				//stalaktyty
				for (int k = 1; k <= ILOSC_STALAKTYTOW; k++) {
					int start_showing = k * ODL_POM_STALAKTYTAMI;
					int end_showing = k * ODL_POM_STALAKTYTAMI + SCREEN_WIDTH + stal.w;


					if (start_showing < frames && frames < end_showing) {

						if (SDL_HasIntersection(&horse, &stal)) {
							ask_if_play_again = true;
							ilosc_zyc--;
							printf("STALAKTYT \n");
						}

						stal.x = start_showing + SCREEN_WIDTH - frames;

						SDL_BlitSurface(stalaktyt, NULL, screen, &stal);
					}
				}
#endif

				// tekst informacyjny / info text
				DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
				sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
				sprintf(text, "Uzyskane puntky: %d", punkty);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);


				//DrawRectangle(screen, horse.x, horse.y, horse.w, horse.h, czerwony, niebieski);
				DrawRectangle(screen, stal.x, stal.y, stal.w, stal.h, czerwony, niebieski);
				DrawRectangle(screen, gwiazda.x, gwiazda.y, gwiazda.w, gwiazda.h, czerwony, niebieski);
				DrawRectangle(screen, girl.x, girl.y, girl.w, girl.h, czerwony, niebieski);

				update_the_screen(screen, scrtex, renderer);

				frames += mnoznik_przyspieszenia;
				dystans += mnoznik_przyspieszenia;
				punkty += mnoznik_przyspieszenia;
				//rozszerzanie mapy
				if (frames > SZEROKOSC_MAPY_BMP) {
					frames = 0;
					mnoznik_przyspieszenia += 1;
					ilosc_dodawanych_klatek_w_zrywie += mnoznik_przyspieszenia;

					for (int k = 1; k <= ILOSC_PRZESZKOD; k++) 
						przeszkoda_zotała_rozbita[k] = 0;

					for (int k = 1; k <= ILOSC_WROZEK; k++) 
						wrozka_zostala_zebrana[k] = 0;

				}


				if (quit == 1) {
					zwolnienie_powierzchni(screen, scrtex, window, renderer);
					return 0;
				}
			}
			else {
				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_KEYDOWN:
						//wyjscie z programu
						if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1; 

						//nowa gra
						else if (event.key.keysym.sym == SDLK_n) {
							ask_if_play_again = false;
							frames = 0;
							jump = 0;
							
							mnoznik_przyspieszenia = 1;
							IsJumping = 0;
							IsFalling = 0;
							jump = 0;
							
							dystans = 0;

							
							kolejna_gwiazda_rozbita = 0;
							for (int k = 1; k <= ILOSC_PRZESZKOD; k++) {
								przeszkoda_zotała_rozbita[k] = 0;
								przeszkoda_ominieta[k] = 0;
							}

							kolejna_wrozka_zebrana = 0;
							for (int k = 1; k <= ILOSC_WROZEK; k++) {
								wrozka_zostala_zebrana[k] = 0; 
								wrozka_ominieta[k] = 0;
							}


							zryw = 5;
							dlugosc_klatek_zrywu = 0;
							ilosc_dodawanych_klatek_w_zrywie = mnoznik_przyspieszenia;


							punkty = 0;

							
							horse.x = SZEROKOSC_KONIA / 2 - 35;
							horse.y = SURFACE_HEIGHT - SZEROKOSC_KONIA / 2 - 25;


							//nie wiadomo czy potrzebne
							gwiazda.x = SCREEN_WIDTH;
							gwiazda.y = 260;

							girl.x = SCREEN_WIDTH;
							girl.y = 180 - 15;

							stal.x = SCREEN_WIDTH;
							stal.y = 40;

							podloze.x = 0;
							podloze.y = SURFACE_HEIGHT + 30;

							hole.x = 0;
							hole.y = SURFACE_HEIGHT + 10;
							
							dodatkowe.x = 0;
							dodatkowe.y = SURFACE_HEIGHT - WYSOKOSC_DODATKOWEGO_PODLOZA + 10;
						}
					break;

					case SDL_QUIT:
						quit = 1;
						begin = 1;
					break;

					};
				};

				DrawSurface(screen, click_to_play, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

				DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
				sprintf(text, "Pozostalych szans na spelnienie marzen: %d.", ilosc_zyc);
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
				sprintf(text, "Wcisnij n zeby zaczac nowa ture gry. ");
				DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);


				update_the_screen(screen, scrtex, renderer);

				if (quit == 1) {
					zwolnienie_powierzchni(screen, scrtex, window, renderer);
					return 0;
				}

			}
		}

		////ilosc zyc ==0 --> przejdz do menu glownego
		begin = 0;

	}

}
