#include<iostream>
#include<SDL.h>
#include<time.h>
#include<thread>
#include<mutex>
#include<vector>
#include<boost/multiprecision/cpp_bin_float.hpp>
using namespace std;
using boost::multiprecision::cpp_bin_float_50;
mutex mu;
struct mieszana {
	cpp_bin_float_50 r;
	cpp_bin_float_50 i;
};
short szerokosc_ekranu = 1920, wysokosc_ekranu = 1080, generuj;
SDL_Window* window = SDL_CreateWindow("Mandelbrot", 0, SDL_WINDOWPOS_CENTERED, szerokosc_ekranu, wysokosc_ekranu, SDL_WINDOW_SHOWN);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
int start;
void render(short x_poczatek, short x_koniec, mieszana gorny_lewy, cpp_bin_float_50 liczba_na_piksel, short max_iteracji, short dalekosc, short wielkosc_piksela) {
	short kolor, iteracje, czerwony, zielony, niebieski, polowa_piksela;
	cpp_bin_float_50 pomoc;
	mieszana c, z;
	polowa_piksela = wielkosc_piksela / 2;
	for (x_poczatek += polowa_piksela; x_poczatek < x_koniec && generuj; x_poczatek += wielkosc_piksela) {
		for (short y_na_ekranie = polowa_piksela; y_na_ekranie < wysokosc_ekranu; y_na_ekranie += wielkosc_piksela) {
			c.r = gorny_lewy.r + x_poczatek * liczba_na_piksel;
			c.i = gorny_lewy.i - y_na_ekranie * liczba_na_piksel;
			z.r = c.r;
			z.i = c.i;
			for (iteracje = 0; iteracje < max_iteracji && z.r<dalekosc && z.r>-dalekosc && z.i<dalekosc && z.i>-dalekosc; iteracje++) {
				pomoc = z.r;
				z.r = z.r * z.r - z.i * z.i + c.r;
				z.i = 2 * pomoc * z.i + c.i;
			}
			//kolory
			if (iteracje != max_iteracji) {
				kolor = iteracje * 5 % 1530;
				switch (kolor / 255) {
				case 0:
					czerwony = 255;
					zielony = kolor;
					niebieski = 0;
					break;
				case 1:
					czerwony = 255 - kolor + 255;
					zielony = 255;
					niebieski = 0;
					break;
				case 2:
					czerwony = 0;
					zielony = 255;
					niebieski = kolor - 510;
					break;
				case 3:
					czerwony = 0;
					zielony = 255 - (kolor - (255 * 3));
					niebieski = 255;
					break;
				case 4:
					czerwony = kolor - (255 * 4);
					zielony = 0;
					niebieski = 255;
					break;
				case 5:
					czerwony = 255;
					zielony = 0;
					niebieski = 255 - (kolor - (255 * 5));
					break;
				}
			}
			else {
				czerwony = 0;
				zielony = 0;
				niebieski = 0;
			}
			mu.lock();
			SDL_SetRenderDrawColor(renderer, czerwony, zielony, niebieski, 255);
			for (short i = x_poczatek - polowa_piksela; i <= x_poczatek + polowa_piksela && i < x_koniec; i++) {
				for (short j = y_na_ekranie - polowa_piksela; j <= y_na_ekranie + polowa_piksela; j++) {
					SDL_RenderDrawPoint(renderer, i, j);
				}
			}
			mu.unlock();
		}
		if (clock() - start > 100) {
			start = clock();
			mu.lock();
			SDL_RenderPresent(renderer);
			mu.unlock();
		}
	}
}
int main(int argc, char* argv[]) {
	bool dziala = true;
	short max_iteracji = 1000, dalekosc=2, watki, czerwony, zielony, niebieski, kolor, iteracje, szerokosc_paska, wielkosc_piksela, polowa_piksela;
	cpp_bin_float_50 liczba_na_piksel, zoom = 4, x = 0, y = 0, pomoc;
	mieszana gorny_lewy, c, z;
	vector<thread> threads;
	watki = thread::hardware_concurrency();
	szerokosc_paska = szerokosc_ekranu / watki;
	SDL_Event event;
	SDL_PollEvent(&event);
	while (dziala) {
		if (szerokosc_ekranu > wysokosc_ekranu) {
			liczba_na_piksel = zoom / (cpp_bin_float_50)wysokosc_ekranu;
		}
		else {
			liczba_na_piksel = zoom / (cpp_bin_float_50)szerokosc_ekranu;
		}
		gorny_lewy.r = x - liczba_na_piksel * szerokosc_ekranu / (cpp_bin_float_50)2;
		gorny_lewy.i = y + liczba_na_piksel * wysokosc_ekranu / (cpp_bin_float_50)2;
		start = clock();
		generuj = true;
		wielkosc_piksela = 1;
		while (wielkosc_piksela*2<szerokosc_paska) {
			wielkosc_piksela = wielkosc_piksela * 2;
		}
		do {
			wielkosc_piksela = wielkosc_piksela / 2;
			polowa_piksela = wielkosc_piksela / 2;
			//danie zajêcia resztom rdzeni
			for (short i = 1; i < watki; i++) {
				threads.push_back(thread(render, szerokosc_paska * i, szerokosc_paska * (i + 1), gorny_lewy, liczba_na_piksel, max_iteracji, dalekosc, wielkosc_piksela));
			}
			//generowanie dla tego rdzenia
			for (short x_poczatek = polowa_piksela; x_poczatek < szerokosc_paska && generuj; x_poczatek += wielkosc_piksela) {
				for (short y_na_ekranie = polowa_piksela; y_na_ekranie < wysokosc_ekranu; y_na_ekranie += wielkosc_piksela) {
					c.r = gorny_lewy.r + x_poczatek * liczba_na_piksel;
					c.i = gorny_lewy.i - y_na_ekranie * liczba_na_piksel;
					z.r = c.r;
					z.i = c.i;
					for (iteracje = 0; iteracje < max_iteracji && z.r<dalekosc && z.r>-dalekosc && z.i<dalekosc && z.i>-dalekosc; iteracje++) {
						pomoc = z.r;
						z.r = z.r * z.r - z.i * z.i + c.r;
						z.i = 2 * pomoc * z.i + c.i;
					}
					//kolory
					if (iteracje != max_iteracji) {
						kolor = iteracje * 5 % 1530;
						switch (kolor / 255) {
						case 0:
							czerwony = 255;
							zielony = kolor;
							niebieski = 0;
						break;
						case 1:
							czerwony = 255 - kolor + 255;
							zielony = 255;
							niebieski = 0;
						break;
						case 2:
							czerwony = 0;
							zielony = 255;
							niebieski = kolor - 510;
						break;
						case 3:
							czerwony = 0;
							zielony = 255 - (kolor - (255 * 3));
							niebieski = 255;
						break;
						case 4:
							czerwony = kolor - (255 * 4);
							zielony = 0;
							niebieski = 255;
						break;
						case 5:
							czerwony = 255;
							zielony = 0;
							niebieski = 255 - (kolor - (255 * 5));
						break;
						}
					}
					else {
						czerwony = 0;
						zielony = 0;
						niebieski = 0;
					}
					//wyswietlanie
					mu.lock();
					SDL_SetRenderDrawColor(renderer, czerwony, zielony, niebieski, 255);
					for (short i = x_poczatek - polowa_piksela; i <= x_poczatek + polowa_piksela && i < szerokosc_paska; i++) {
						for (short j = y_na_ekranie - polowa_piksela; j <= y_na_ekranie + polowa_piksela; j++) {
							SDL_RenderDrawPoint(renderer, i, j);
						}
					}
					mu.unlock();
				}
				//sprawdzenie wciœniêcia przyciksu
				if (clock() - start > 1000) {
					start = clock();
					mu.lock();
					SDL_RenderPresent(renderer);
					mu.unlock();
				}
				SDL_PollEvent(&event);
				if (event.type == SDL_KEYDOWN) {
					mu.lock();
					generuj = false;
					mu.unlock();
				}
			}
			//oczekiwanie na reszte rdzeni
			for (thread& t : threads) {
				if (t.joinable()) {
					t.join();
				}
			}
			SDL_RenderPresent(renderer);
		} while (wielkosc_piksela!=1);
		//oczekiwanie na input
		if (generuj) {
			do {
				SDL_PollEvent(&event);
			} while (event.type != SDL_KEYDOWN);
		}
		//input
		if (event.type == SDL_QUIT) {
			dziala = false;
		}
		else if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				dziala = false;
				break;
			case SDLK_i:
				zoom = zoom / (cpp_bin_float_50)2;
				break;
			case SDLK_o:
				zoom = zoom * 2;
				break;
			case SDLK_DOWN:
				y -= zoom / (cpp_bin_float_50)2;
				break;
			case SDLK_UP:
				y += zoom / (cpp_bin_float_50)2;
				break;
			case SDLK_RIGHT:
				x += zoom / (cpp_bin_float_50)2;
				break;
			case SDLK_LEFT:
				x -= zoom / (cpp_bin_float_50)2;
				break;
			case SDLK_s:
				y -= zoom / (cpp_bin_float_50)8;
				break;
			case SDLK_w:
				y += zoom / (cpp_bin_float_50)8;
				break;
			case SDLK_d:
				x += zoom / (cpp_bin_float_50)8;
				break;
			case SDLK_a:
				x -= zoom / (cpp_bin_float_50)8;
				break;
			case SDLK_KP_PLUS:
				max_iteracji += 100;
				break;
			case SDLK_KP_MINUS:
				max_iteracji -= 100;
				break;
			}
		}
	}
	return 0;
}