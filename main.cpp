#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

using namespace std;

bool is_grayscale_image(SDL_Surface *image){
        Uint8 *image_pixels = (Uint8*)image->pixels;
        SDL_PixelFormat image_pixel_format = image->format;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(image_pixel_format);
        Uint8 bytes_per_pixel = format_details->bytes_per_pixel;

        for(int i = 0; i < image->h; i++){
                for(int j = 0; j < image->w; j++){
                        Uint8* src = (Uint8*)image_pixels + i*image->pitch + j*bytes_per_pixel;

                        Uint8 r = src[0], g = src[1], b = src[2];

                        if(!(r == g && g == b)) return false;
                }
        }
        return true;
}

SDL_Surface* to_grayscale(SDL_Surface* src_image){
        Uint8 *image_pixels = (Uint8*)src_image->pixels;
        SDL_PixelFormat src_image_pixel_format = src_image->format;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(src_image_pixel_format);
        Uint8 bytes_per_pixel = format_details->bytes_per_pixel;

        SDL_Surface *grayscale_image = SDL_CreateSurface(src_image->w, src_image->h, src_image_pixel_format);

        Uint8 *output_pixels = (Uint8*)grayscale_image->pixels;

        SDL_PixelFormat grayscale_image_pixel_format = grayscale_image->format;
        const SDL_PixelFormatDetails *output_format_details = SDL_GetPixelFormatDetails(grayscale_image_pixel_format);

        for(int i = 0; i < src_image->h; i++){
                for(int j = 0; j < src_image->w; j++){
                        Uint8* src = (Uint8*)src_image->pixels + i*src_image->pitch + j*bytes_per_pixel;
                        Uint8* dst = (Uint8*)grayscale_image->pixels + i*grayscale_image->pitch + j*bytes_per_pixel;

                        const Uint8 color = 0.2125*src[0] + 0.7154*src[1] + 0.0721*src[2];

                        dst[0] = dst[1] = dst[2] = color;
                }
        }

        return grayscale_image;
}



int main(int argc, char** argv){
        if(argc != 2){
                cerr << "Número de argumentos inválido! Insira exatamente uma imagem." << endl;
                return 1;
        }

        const char* source_code_filename = argv[1];

        SDL_Surface *input_image;
        input_image = IMG_Load(source_code_filename);

        SDL_Surface* input_image24 = SDL_ConvertSurface(input_image, SDL_PIXELFORMAT_RGB24);

        if(!input_image){
                cerr << "Erro ao abrir o arquivo." << endl;
                return 1;
        }

        if(!is_grayscale_image(input_image)){
                SDL_Surface* output_image = to_grayscale(input_image24);

                SDL_SaveBMP(output_image, "grayscale_image.bmp");
                SDL_DestroySurface(output_image);
        } else cout << "Imagem já está em tons de cinza" << endl;

        SDL_DestroySurface(input_image);

        return 0;
}
