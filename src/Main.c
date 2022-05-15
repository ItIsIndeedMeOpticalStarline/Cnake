#include "SDL.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Creates assertion. Takes: condition, (const char*)error message
#define EVE_ASSERT(con, str) EveAssert(#con, con, __FILE__, __LINE__, str) // <- Copied from my lib

void EveAssert(const char* conStr, bool con, const char* file, int lin, const char* str) // <- Copied from my lib
{
	if (!con)
	{
		fprintf_s(stderr, "Assertion failed: %s: \nExpected %s\nin %s, at line %i", str, conStr, file, lin);
		exit(13);
	}
}

#define WINDOW_WIDTH 800
#define	WINDOW_HEIGHT 600

#define SQUARE_SIZE 50

#define BOARD_WIDTH  (WINDOW_WIDTH)  / (SQUARE_SIZE)
#define BOARD_HEIGHT (WINDOW_HEIGHT) / (SQUARE_SIZE)

enum Keys
{
	K_UP,
	K_LEFT,
	K_DOWN,
	K_RIGHT,
	K_ESCAPE,

	K_NUM_KEYS
};

static bool keys[5] = { false };

typedef struct
{
	int8_t x, y;
} pos_t;

pos_t Pos_Add(pos_t a, pos_t b)
{
	return (pos_t){ a.x + b.x, a.y + b.y };
}

pos_t Pos_Sub(pos_t a, pos_t b)
{
	return (pos_t) { a.x - b.x, a.y - b.y };
}

bool Pos_Eq(pos_t a, pos_t b)
{
	if (a.x == b.x && a.y == b.y)
		return true;

	return false;
}

#define SNAKE_MAX_LENGTH 200

typedef struct
{
	pos_t head;
	uint8_t bodyIndex;
	pos_t facing;
	pos_t body[SNAKE_MAX_LENGTH];
} snake_t;

enum Entities
{
	E_NONE,
	E_SNAKE,
	E_APPLE,

	NUM_ENTITIES
};

const pos_t DIR_NONE	= { 0,  0 };
const pos_t DIR_NORTH	= { 0, -1 };
const pos_t DIR_EAST	= { 1,  0 };
const pos_t DIR_SOUTH	= { 0,  1 };
const pos_t DIR_WEST	= { -1, 0 };

int main(int argc, char* argv[])
{
	EVE_ASSERT(SDL_Init(SDL_INIT_VIDEO) >= 0, "SDL2 failed to initalize");

	SDL_Window* window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0x0);
	EVE_ASSERT(window != NULL, "SDL2 failed to create window");

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	EVE_ASSERT(renderer != NULL, "SDL2 failed to create renderer");

	SDL_Event event;

	uint64_t gameTick = SDL_GetTicks();
	double accumulator = 0.0;
	double deltaTime = 0.1; 
	double frameTime, newTime;
	double presentTime = SDL_GetTicks() / 1000.0;

	srand((unsigned int)time(0));

	uint8_t board[BOARD_WIDTH][BOARD_HEIGHT];

	for (size_t y = 0; y < BOARD_HEIGHT; y++)
	{
		for (size_t x = 0; x < BOARD_WIDTH; x++)
		{
			board[x][y] = E_NONE;
		}
	}

	snake_t snake = { { BOARD_WIDTH / 2, BOARD_HEIGHT / 2 }, 3, DIR_WEST };

	for (size_t i = 0; i < snake.bodyIndex; i++)
	{
		snake.body[i] = (pos_t){ snake.head.x + (int8_t)i, snake.head.y };
		board[snake.body[i].x][snake.body[i].y] = E_SNAKE;
	}

	pos_t newApple = { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };
	while (board[newApple.x][newApple.y] != E_NONE)  // I'm aware that this is a terrible way to do this
	{
		newApple = (pos_t){ rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };
	}
	board[newApple.x][newApple.y] = E_APPLE;

	bool stop = false;
	bool quit = false;
	while (!quit)
	{
		gameTick = SDL_GetTicks();
		newTime = SDL_GetTicks() / 1000.0;
		frameTime = newTime - presentTime;
		presentTime = newTime;
		accumulator += frameTime;

		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
				case SDL_QUIT: { quit = true; } break;
				case SDL_KEYDOWN: 
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_UP:		{ keys[K_UP] = true; }		break;
						case SDLK_LEFT:		{ keys[K_LEFT] = true; }	break;
						case SDLK_DOWN:		{ keys[K_DOWN] = true; }	break;
						case SDLK_RIGHT:	{ keys[K_RIGHT] = true; }	break;
						case SDLK_ESCAPE:	{ keys[K_ESCAPE] = true; }	break;
					}
				} break;
				case SDL_KEYUP:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_UP:		{ keys[K_UP] = false; }		break;
						case SDLK_LEFT:		{ keys[K_LEFT] = false; }	break;
						case SDLK_DOWN:		{ keys[K_DOWN] = false; }	break;
						case SDLK_RIGHT:	{ keys[K_RIGHT] = false; }	break;
						case SDLK_ESCAPE:	{ keys[K_ESCAPE] = false; } break;
					}
				} break;
			}
		}

		if (keys[K_ESCAPE])
		{
			SDL_Event quitEvent;
			quitEvent.type = SDL_QUIT;
			SDL_PushEvent(&quitEvent);
		}

		if (!stop)
		{
			if (keys[K_UP] && !Pos_Eq(snake.facing, DIR_SOUTH))
				snake.facing = DIR_NORTH;
			else if (keys[K_LEFT] && !Pos_Eq(snake.facing, DIR_EAST))
				snake.facing = DIR_WEST;
			else if (keys[K_DOWN] && !Pos_Eq(snake.facing, DIR_NORTH))
				snake.facing = DIR_SOUTH;
			else if (keys[K_RIGHT] && !Pos_Eq(snake.facing, DIR_WEST))
				snake.facing = DIR_EAST;
		}

		while (accumulator >= deltaTime)
		{
			if (!stop)
			{
				for (size_t i = snake.bodyIndex; i > 0; i--)
				{
					size_t idx = i - 1;
					if (idx != 0) // body
					{
						if (idx == snake.bodyIndex - 1)
						{
							board[snake.body[idx].x][snake.body[idx].y] = E_NONE;
						}

						snake.body[idx] = snake.body[idx - 1];
					}
					else // head
					{
						snake.head = Pos_Add(snake.head, snake.facing);

						if (snake.head.x < 0)
							snake.head.x = BOARD_WIDTH - 1;

						if (snake.head.x >= BOARD_WIDTH)
							snake.head.x = 0;

						if (snake.head.y < 0)
							snake.head.y = BOARD_HEIGHT - 1;

						if (snake.head.y >= BOARD_HEIGHT)
							snake.head.y = 0;

						snake.body[idx] = snake.head;

						if (board[snake.head.x][snake.head.y] == E_APPLE)
						{
							if (snake.bodyIndex < SNAKE_MAX_LENGTH)
							{
								snake.body[snake.bodyIndex + 1] = Pos_Add(snake.body[snake.bodyIndex], Pos_Sub(snake.body[snake.bodyIndex], snake.body[snake.bodyIndex - 1]));
								snake.bodyIndex++;

								pos_t newApple = { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };  // Copypasted code! yuck!
								while (board[newApple.x][newApple.y] != E_NONE)
								{
									newApple = (pos_t){ rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };
								}
								board[newApple.x][newApple.y] = E_APPLE;
							}

							if (snake.bodyIndex >= SNAKE_MAX_LENGTH) // not else so win triggers after last apple pickup
							{
								printf_s("You win!"); // No text support in pure SDL :(
								snake.facing = DIR_NONE;
								stop = true;
							}
						}

						if (board[snake.head.x][snake.head.y] == E_SNAKE)
						{
							printf_s("You lose!"); // No text support in pure SDL :(
							snake.facing = DIR_NONE;
							stop = true;
						}
					}
					board[snake.body[idx].x][snake.body[idx].y] = E_SNAKE;
				}
			}

			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
			SDL_RenderClear(renderer);

			for (size_t y = 0; y < BOARD_HEIGHT; y++)
			{
				for (size_t x = 0; x < BOARD_WIDTH; x++)
				{
					switch (board[x][y])
					{
						case E_NONE: { continue; } break;
						case E_SNAKE:
						{
							SDL_Rect rect = { (int)x * SQUARE_SIZE, (int)y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE };
							SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
							SDL_RenderFillRect(renderer, &rect);
						} break;
						case E_APPLE:
						{
							SDL_Rect rect = { (int)x * SQUARE_SIZE, (int)y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE };
							SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE);
							SDL_RenderFillRect(renderer, &rect);
						} break;
						default:
						{
							fprintf_s(stderr, "Unknown entity type");
							exit(1);
						}
					}
				}
			}

			SDL_RenderPresent(renderer);

			accumulator -= deltaTime;
		}
	}

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}