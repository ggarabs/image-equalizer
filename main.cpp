#include <vector>
#include <iostream>
#include <unistd.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

using namespace std;

typedef struct Histogram {
        SDL_FRect area;
        vector<int> values;
        int total_bits;
        int max_value;
} Histogram;

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

vector <int> get_pixels_counting_by_intensity(SDL_Surface* src_image){
        Uint8 *image_pixels = (Uint8*)src_image->pixels;
        SDL_PixelFormat src_image_pixel_format = src_image->format;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(src_image_pixel_format);
        Uint8 bytes_per_pixel = format_details->bytes_per_pixel;

        vector <int> response(256, 0);

        for(int i = 0; i < src_image->h; i++){
                for(int j = 0; j < src_image->w; j++){
                        Uint8* src = (Uint8*)src_image->pixels + i*src_image->pitch + j*bytes_per_pixel;

                        const Uint8 color = src[0];

                        response[color]++;
                }
        }

        return response;
}

double get_mean_intensity_by_histogram(vector <int> &histogram){
        double answer = 0.0;
        long histogram_bits = 0;

        for(int i = 0; i < (int)histogram.size(); i++){
                answer += i*histogram[i];
                histogram_bits += histogram[i];
        }

        answer /= histogram_bits;

        return answer;
}

void detect_image_brightness(vector<int> &histogram_values) {
        double image_mean_intensity = get_mean_intensity_by_histogram(histogram_values);

        if(image_mean_intensity < 256/3) cout << "Imagem clara" << endl;
        else if(image_mean_intensity <= 2*256/3) cout << "Imagem média" << endl;
        else cout << "Imagem escura" << endl;
}

int get_max_intensity_ocurrence_by_histogram(const vector<int> &histogram_values) {
        int max_value = 0;
        for (int i = 0; i < (int) histogram_values.size(); i++) 
                max_value = histogram_values[i] > max_value ? histogram_values[i] : max_value;
        return max_value;
}

int get_total_bits_from_histogram(const vector<int> &histogram_values) {
        int total_bits = 0;
        for (int i = 0; i < (int) histogram_values.size(); i++) 
                total_bits += histogram_values[i];
        return total_bits;
}

Histogram* create_image_histogram(SDL_Surface *image) {
        Histogram *histogram = new Histogram;

        histogram->values = get_pixels_counting_by_intensity(image);
        histogram->total_bits = get_total_bits_from_histogram(histogram->values);
        histogram->max_value = get_max_intensity_ocurrence_by_histogram(histogram->values);
        histogram->area = {0, 0, 256.0f * 3, 256.0f * 2};

        return histogram;
}

void render_histogram(SDL_Renderer *renderer, SDL_Window *window, Histogram *histogram) {
        int window_w, window_h;
        SDL_GetWindowSize(window, &window_w, &window_h);

        const float x = (window_w - histogram->area.w) * 0.5f;
        const float y = (window_h - histogram->area.h) * 0.5f;
        const SDL_FRect histogram_area = {x, y, histogram->area.w, histogram->area.h};

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderFillRect(renderer, &histogram_area);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderLine(renderer, x, y, x, y + histogram->area.h);  // left line
        SDL_RenderLine(renderer, x + histogram->area.w, y, x + histogram->area.w, y + histogram->area.h);  // right line

        SDL_RenderLine(renderer, x, y + histogram->area.h, x + histogram->area.w, y + histogram->area.h);  // lower line
        SDL_RenderLine(renderer, x, y, x + histogram->area.w, y);  // upper line

        float max_pixel_ration = (float) histogram->max_value / histogram->total_bits;

        for (int i = 0; i < (int) histogram->values.size(); i++) {
                float bar_x = x + (i * (histogram->area.w / 256.0f));
                float bar_y = y + histogram->area.h;
                float bar_w = histogram->area.w / 256.0f - 1;

                float pixel_ratio = (float) histogram->values[i] / histogram->total_bits;
                float bar_h = (-pixel_ratio * 0.9f / max_pixel_ration) * histogram->area.h;

                SDL_FRect bar = {bar_x, bar_y, bar_w, bar_h};
                SDL_RenderFillRect(renderer, &bar);
        }
}

SDL_FRect create_button_text(SDL_FRect button) {
        float text_w = button.x * 3.0f / 5.0f;
        float text_h = button.h * 3.0f / 5.0f;
        float text_x = button.x + button.x / 5.0f;
        float text_y = button.y + button.h / 5.0f;

        return {text_x, text_y, text_w, text_h};
}

SDL_FRect create_button(SDL_Window *window) {
        int secondary_window_w, secondary_window_h;
        SDL_GetWindowSize(window, &secondary_window_w, &secondary_window_h);

        float button_w = 250;
        float button_h = 50;
        float button_x = (secondary_window_w - button_w) * 0.5f;
        float button_y = secondary_window_h * 4.0f / 5.0f;
        
        return {button_x, button_y, button_w, button_h};
}

void render_button(SDL_Window *window, SDL_Renderer *renderer, SDL_FRect &button, SDL_FRect &text_rect, const char* button_text, TTF_Font *font, SDL_Color text_color) {
        int secondary_window_w, secondary_window_h;
        SDL_GetWindowSize(window, &secondary_window_w, &secondary_window_h);

        button.x = (secondary_window_w - button.w) * 0.5f;
        button.y = secondary_window_h * 4.0f / 5.0f;

        text_rect.h = button.h * 3.0f / 5.0f;
        text_rect.w = button.w * 3.0f / 5.0f;
        text_rect.x = button.x + button.w / 5.0f;
        text_rect.y = button.y + button.h / 5.0f;
        
        SDL_Surface *text_surface = TTF_RenderText_Solid(font, button_text , 9*sizeof(char), text_color);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        
        SDL_RenderFillRect(renderer, &button);
        SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);

        SDL_DestroyTexture(text_texture);
        SDL_DestroySurface(text_surface);
}

int main(int argc, char** argv) {
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

        } else 
                cout << "Imagem já está em tons de cinza" << endl;

        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();

        int secondary_window_w = 1000, secondary_window_h = 1000;

        SDL_Window* main_window = SDL_CreateWindow("Imagem original", input_image->w, input_image->h, SDL_WINDOW_RESIZABLE);
        SDL_Window* secondary_window = SDL_CreateWindow("Histograma", secondary_window_w, secondary_window_h, SDL_WINDOW_RESIZABLE);

        SDL_SetWindowParent(secondary_window, main_window);

        if(!main_window) cout << "Erro ao abrir a janela" << endl;
        if(!secondary_window) cout << "Erro ao abrir janela secundária" << endl;

        SDL_Renderer* renderer = SDL_CreateRenderer(secondary_window, NULL);
        SDL_Surface* window_surface = SDL_GetWindowSurface(main_window);

        SDL_SetWindowPosition(main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowPosition(secondary_window, SDL_WINDOWPOS_CENTERED-output_image->w/2, SDL_WINDOWPOS_CENTERED);

        SDL_FRect button = create_button(secondary_window);
        SDL_FRect text_rect = create_button_text(button);
        Histogram *histogram = create_image_histogram(output_image);

        TTF_Font *font = TTF_OpenFont("./fonts/BitcountGrid.ttf", 1000);
        if(!font){
                cerr << "Erro ao carregar a fonte" << endl;
                return 1;
        }

        const Uint32 main_window_id = SDL_GetWindowID(main_window);
        const Uint32 secondary_window_id = SDL_GetWindowID(secondary_window);

        bool mode = false;
        bool done = false, button_pressed = false;
        const char* button_texts[] = {"Equalizar", "Restaurar"};
        
        SDL_Color text_color = {255, 255, 255, 255};

        while(!done) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);

                SDL_Event event;

                while(SDL_PollEvent(&event)){
                        if(event.type == SDL_EVENT_QUIT) 
                                done = true;

                        else if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                                if(event.window.windowID == secondary_window_id) 
                                        SDL_DestroyWindow(secondary_window);
                        }
                        else if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN){
                                int mouse_x = event.button.x, mouse_y = event.button.y;
                                if(mouse_x >= button.x && 
                                        mouse_x <= button.x + button.w && 
                                        mouse_y >= button.y && 
                                        mouse_y <= button.y + button.h){
                                                button_pressed = true;
                                                mode = !mode;
                                        }
                        } 
                        else if(event.type == SDL_EVENT_MOUSE_BUTTON_UP) 
                                button_pressed = false;

                        else if (event.type == SDL_EVENT_KEY_DOWN) {
                                if (event.key.key == SDLK_S)
                                        SDL_SaveBMP(output_image, "output_image.png");
                        }
                }

                float mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);

                if(button_pressed) SDL_SetRenderDrawColor(renderer, 1, 31, 75, 255);
                else if(mouse_x >= button.x && 
                                        mouse_x <= button.x + button.w && 
                                        mouse_y >= button.y && 
                                        mouse_y <= button.y + button.h) SDL_SetRenderDrawColor(renderer, 100, 151, 177, 255);
                else SDL_SetRenderDrawColor(renderer, 0, 91, 150, 255);

                render_button(secondary_window, renderer, button, text_rect, button_texts[mode], font, text_color);
                render_histogram(renderer, secondary_window, histogram);

                SDL_BlitSurface(output_image, NULL, window_surface, NULL);
                SDL_UpdateWindowSurface(main_window);
                SDL_RenderPresent(renderer);
        }

        TTF_CloseFont(font);
        TTF_Quit();

        SDL_DestroyWindow(main_window);
        SDL_DestroyRenderer(renderer);

        SDL_DestroyWindow(secondary_window);
        SDL_DestroySurface(input_image);
        SDL_DestroySurface(output_image);
        SDL_DestroySurface(input_image24);
        
        SDL_Quit();

        return 0;
}
