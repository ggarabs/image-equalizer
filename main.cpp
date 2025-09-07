#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

using namespace std;

int main(int argc, char** argv){
        if(argc != 2){
                cerr << "Número de argumentos inválido! Insira exatamente uma imagem." << endl;
                return 1;
        }

        const char* source_code_filename = argv[1];

        SDL_Surface *input_image;
        input_image = IMG_Load(source_code_filename);

        if(!input_image){
                cerr << "Erro ao abrir o arquivo." << endl;
                return 1;
        }

        SDL_Surface* input_image24 = SDL_ConvertSurface(input_image, SDL_PIXELFORMAT_RGB24);
        Uint8 *input_image24_image_pixels = (Uint8*)input_image24->pixels;

        SDL_PixelFormat input_pixel_format = input_image24->format;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(input_pixel_format);
        
        Uint8 bytes_per_pixel = format_details->bytes_per_pixel;

        SDL_Surface *black_and_white_image = SDL_CreateSurface(input_image24->w, input_image24->h, input_pixel_format);

        Uint8 *output_pixels = (Uint8*)black_and_white_image->pixels;

        SDL_PixelFormat black_and_white_image_pixel_format = black_and_white_image->format;
        const SDL_PixelFormatDetails *output_format_details = SDL_GetPixelFormatDetails(black_and_white_image_pixel_format);

        for(int i = 0; i < input_image24->h; i++){
                for(int j = 0; j < input_image24->w; j++){
                        Uint8* src = (Uint8*)input_image24->pixels + i*input_image24->pitch + j*bytes_per_pixel;
                        Uint8* dst = (Uint8*)black_and_white_image->pixels + i*black_and_white_image->pitch + j*bytes_per_pixel;

                        const Uint8 color = 0.2125*src[0] + 0.7154*src[1] + 0.0721*src[2];

                        dst[0] = dst[1] = dst[2] = color;
                }
        }

        SDL_SaveBMP(black_and_white_image, "black_and_white_image.bmp");

        SDL_DestroySurface(input_image);

        return 0;
}
