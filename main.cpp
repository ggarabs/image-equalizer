#include <iostream>
#include <unistd.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_video.h>

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

        if(!input_image){
                cerr << "Erro ao abrir o arquivo." << endl;
                return 1;
        }
        
        SDL_Surface* input_image24 = SDL_ConvertSurface(input_image, SDL_PIXELFORMAT_RGB24);

        SDL_Surface* output_image = input_image;

        if(!is_grayscale_image(input_image)){
                output_image = to_grayscale(input_image24);

                SDL_SaveBMP(output_image, "grayscale_image.bmp");

        } else cout << "Imagem já está em tons de cinza" << endl;

        pid_t pid = fork();

        if(pid == 0){
                SDL_Init(SDL_INIT_VIDEO);

                SDL_Window* secondary_window = SDL_CreateWindow("Histograma", 500, 500, SDL_WINDOW_INPUT_FOCUS);

                SDL_SetWindowPosition(secondary_window, SDL_WINDOWPOS_CENTERED-10, SDL_WINDOWPOS_CENTERED-10);

                if(!secondary_window) cout << "Erro ao abrir janela secundária" << endl;

                SDL_Surface* secondary_window_surface = SDL_GetWindowSurface(secondary_window);

                if(!secondary_window_surface){
                        cout << SDL_GetError() << endl;
                }

                bool done = false;

                while(!done){
                        SDL_Event event;

                        while(SDL_PollEvent(&event)){
                                if(event.type == SDL_EVENT_QUIT){
                                        done = true;
                                }
                        }
                }

                SDL_DestroyWindow(secondary_window);
                SDL_DestroySurface(secondary_window_surface);

        }else if(pid > 0){
                SDL_Init(SDL_INIT_VIDEO);

                SDL_Window* main_window = SDL_CreateWindow("Imagem original", input_image->w, input_image->h, SDL_WINDOW_BORDERLESS);

                SDL_SetWindowPosition(main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

                if(!main_window) cout << "Erro ao abrir a janela" << endl;

                SDL_Surface* window_surface = SDL_GetWindowSurface(main_window);

                if(!window_surface){
                        cout << SDL_GetError() << endl;
                }

                bool done = false;

                while(!done){
                        SDL_Event event;

                        while(SDL_PollEvent(&event)){
                                if(event.type == SDL_EVENT_QUIT){
                                        done = true;
                                }
                        }

                        SDL_BlitSurface(output_image, NULL, window_surface, NULL);

                        SDL_UpdateWindowSurface(main_window);
                }

                SDL_DestroyWindow(main_window);
                SDL_DestroySurface(window_surface);

        }else perror("Falha na exibição da nova janela");

        SDL_DestroySurface(input_image);
        SDL_DestroySurface(output_image);
        SDL_DestroySurface(input_image24);

        return 0;
}
