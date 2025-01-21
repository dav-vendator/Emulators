#include <SDL.h>

class Platform{
    SDL_Window* window{};
    SDL_Renderer* renderer{};
    SDL_Texture* texture{};
    public:
        Platform(const char*, int,  int , int, int);
        ~Platform();
        void update(void const*, int);
        bool processInput(uint8_t * );      
};