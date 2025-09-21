#include <math.h>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <unistd.h>
#include <algorithm>
#include <filesystem>
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

bool file_exists(const char* fileName) {
        return std::filesystem::exists(fileName);
}

bool supported_format(const char* fileName) {
        // Got this list based on SDL_image supported formats
        // Basically, SDL_image checks the file for each of these extensions
        // See: https://github.com/libsdl-org/SDL_image/blob/main/src/IMG.c#L52

        SDL_IOStream *stream = SDL_IOFromFile(fileName, "rb");

        return IMG_isAVIF(stream) || IMG_isBMP(stream) || IMG_isCUR(stream) || 
               IMG_isICO(stream) || IMG_isGIF(stream) || IMG_isJPG(stream) || 
               IMG_isJXL(stream) || IMG_isLBM(stream) || IMG_isPCX(stream) || 
               IMG_isPNG(stream) || IMG_isPNM(stream) || IMG_isSVG(stream) ||
               IMG_isTIF(stream) || IMG_isXCF(stream) || IMG_isXPM(stream) ||
               IMG_isXCF(stream) || IMG_isXPM(stream) || IMG_isXV(stream) ||
               IMG_isWEBP(stream) || IMG_isQOI(stream);
}

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

double get_mean_intensity_from_histogram(const vector <int> &histogram){
        double answer = 0.0;
        long histogram_bits = 0;

        for(int i = 0; i < (int)histogram.size(); i++){
                answer += i*histogram[i];
                histogram_bits += histogram[i];
        }

        answer /= histogram_bits;

        return answer;
}

double get_standard_deviation_from_histogram(const vector <int> &histogram){
        double answer = 0.0, mean = get_mean_intensity_from_histogram(histogram);
        long histogram_bits = 0;

        for(int i = 0; i < (int)histogram.size(); i++){
                answer += histogram[i]*pow(i-mean, 2);
                histogram_bits += histogram[i];
        }

        answer = sqrt(answer/histogram_bits);

        return answer;
}

string detect_image_brightness(const vector<int> &histogram_values) {
        double image_mean_intensity = get_mean_intensity_from_histogram(histogram_values);

        if(image_mean_intensity < 256/3) return "Clara";
        else if(image_mean_intensity <= 2*256/3) return "Média";
        else return "Escura";
}

string detect_image_contrast(const vector<int> &histogram_values){
        double image_standard_deviation = get_standard_deviation_from_histogram(histogram_values);

        if(image_standard_deviation <= 40) return "Baixo";
        else if(image_standard_deviation <= 80) return "Médio";
        else return "Alto";
}

int get_max_intensity_ocurrence_from_histogram(const vector<int> &histogram_values) {
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
        histogram->max_value = get_max_intensity_ocurrence_from_histogram(histogram->values);
        histogram->area = {0, 0, 256.0f * 2, 256.0f * 1};

        return histogram;
}

void render_text(SDL_Renderer *renderer, float x, float y, TTF_Font *font, string text, SDL_Color text_color) {
        SDL_FRect text_area = {x, y, (int) text.size() * 10.0f, 24.0f};
        SDL_Surface *text_surface = TTF_RenderText_Solid(font, text.c_str() , text.size(), text_color);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        
        SDL_RenderTexture(renderer, text_texture, NULL, &text_area);

        SDL_DestroyTexture(text_texture);
        SDL_DestroySurface(text_surface);
}

void render_histogram(SDL_Renderer *renderer, SDL_Window *window, Histogram *histogram, TTF_Font *font, SDL_Color text_color) {
        int window_w, window_h;
        SDL_GetWindowSize(window, &window_w, &window_h);

        const float x = (window_w - histogram->area.w + 150) * 0.5f;
        const float y = (window_h - histogram->area.h) * 0.5f;
        const SDL_FRect histogram_area = {x, y, histogram->area.w, histogram->area.h};

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderFillRect(renderer, &histogram_area);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        string title = "Histograma";
        int title_w = (int) title.size() * 10.0f, title_h = 30;
        render_text(renderer, (window_w - title_w + 150) * 0.5f, y - 30, font, "Histograma", text_color);

        render_text(renderer, x - 150, y, font, "Intensidade:", text_color);
        render_text(renderer, x - 150, y + 40, font, detect_image_brightness(histogram->values), text_color);

        render_text(renderer, x - 150, y + 110, font, "Contraste:", text_color);
        render_text(renderer, x - 150, y + 150, font, detect_image_contrast(histogram->values), text_color);
        
        SDL_RenderLine(renderer, x, y, x, y + histogram->area.h);
        SDL_RenderLine(renderer, x + histogram->area.w, y, x + histogram->area.w, y + histogram->area.h); 

        SDL_RenderLine(renderer, x, y + histogram->area.h, x + histogram->area.w, y + histogram->area.h);
        SDL_RenderLine(renderer, x, y, x + histogram->area.w, y);

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

map<int, int> get_mapped_bits(Histogram* src_histogram){
        long image_total_bits = src_histogram->total_bits;
        const vector <int> intensity = src_histogram->values;

        vector <double> occurence_probabilities;

        for(int i = 0; i < (int) intensity.size(); i++) occurence_probabilities.push_back((double)intensity[i]/image_total_bits);

        double current_intensity = 0.0;

        map<int, int> mapping_function;

        for(int i = 0; i < (int) intensity.size(); i++){
                current_intensity += 255*occurence_probabilities[i];
                mapping_function[i] = round(current_intensity);
        }

        return mapping_function;
}

Histogram* equalize_histogram(Histogram* src_histogram){
        map<int, int> mapping_function = get_mapped_bits(src_histogram);
        const vector <int> intensity = src_histogram->values;

        int new_max_value = 0;

        vector <int> equalized_intensity(256, 0);

        for(int i = 0; i < (int) intensity.size(); i++){
                equalized_intensity[mapping_function[i]] += intensity[i];
                new_max_value = max(new_max_value, equalized_intensity[mapping_function[i]]);
        }

        Histogram* dest_histogram = new Histogram;

        dest_histogram->area = src_histogram->area;
        dest_histogram->values = equalized_intensity;
        dest_histogram->max_value = new_max_value;
        dest_histogram->total_bits = src_histogram->total_bits;

        return dest_histogram;
}

SDL_Surface* equalize_image(SDL_Surface* src_image, map<int, int> mapping_function){
        Uint8 *image_pixels = (Uint8*)src_image->pixels;
        SDL_PixelFormat src_image_pixel_format = src_image->format;
        const SDL_PixelFormatDetails *format_details = SDL_GetPixelFormatDetails(src_image_pixel_format);
        Uint8 bytes_per_pixel = format_details->bytes_per_pixel;

        SDL_Surface* equalized_image = SDL_CreateSurface(src_image->w, src_image->h, src_image->format);

        for(int i = 0; i < equalized_image->h; i++){
                for(int j = 0; j < equalized_image->w; j++){
                        Uint8* src = (Uint8*)src_image->pixels + i*src_image->pitch + j*bytes_per_pixel;
                        Uint8* dest = (Uint8*)equalized_image->pixels + i*equalized_image->pitch + j*bytes_per_pixel;

                        dest[0] = dest[1] = dest[2] = mapping_function[src[0]];
                }
        }

        return equalized_image;
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

        float button_w = 200;
        float button_h = 40;
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

        if (!file_exists(source_code_filename)){
                cerr << "Arquivo não encontrado!" << endl;
                return 1;
        }

        if (!supported_format(source_code_filename)){
                cerr << "Formato de imagem não suportado!" << endl;
                return 1;
        }

        SDL_Surface *input_image;
        input_image = IMG_Load(source_code_filename);

        if(!input_image){
                cerr << "Erro ao abrir o arquivo." << endl;
                return 1;
        }
        
        SDL_Surface* input_image24 = SDL_ConvertSurface(input_image, SDL_PIXELFORMAT_RGB24);
        SDL_Surface* grayscale_input_image = input_image24;

        if(!is_grayscale_image(input_image24)){
                grayscale_input_image = to_grayscale(input_image24);

                IMG_SavePNG(grayscale_input_image, "./images/output/grayscale_input_image.png");
        } else 
                cout << "Imagem já está em tons de cinza" << endl;

        SDL_Init(SDL_INIT_VIDEO);
        TTF_Init();

        int secondary_window_w = 740, secondary_window_h = 580;

        SDL_Window* main_window = SDL_CreateWindow("Imagem original", input_image->w, input_image->h, 0);
        SDL_Window* secondary_window = SDL_CreateWindow("Histograma", secondary_window_w, secondary_window_h, 0);

        SDL_SetWindowParent(secondary_window, main_window);

        if(!main_window) cout << "Erro ao abrir a janela" << endl;
        if(!secondary_window) cout << "Erro ao abrir janela secundária" << endl;

        SDL_Renderer* renderer = SDL_CreateRenderer(secondary_window, NULL);
        SDL_Surface* window_surface = SDL_GetWindowSurface(main_window);

        SDL_SetWindowPosition(main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_SetWindowPosition(secondary_window, SDL_WINDOWPOS_CENTERED-grayscale_input_image->w/2, SDL_WINDOWPOS_CENTERED);

        SDL_FRect button = create_button(secondary_window);
        SDL_FRect text_rect = create_button_text(button);
        Histogram *histogram = create_image_histogram(grayscale_input_image);
        Histogram *equalized_histogram = equalize_histogram(histogram);

        map<int, int> mapping_function = get_mapped_bits(histogram);

        SDL_Surface* equalized_image = equalize_image(grayscale_input_image, mapping_function);

        cout << get_mean_intensity_from_histogram(histogram->values) << endl;
        cout << get_standard_deviation_from_histogram(histogram->values) << endl;
        detect_image_brightness(histogram->values);
        detect_image_contrast(histogram->values);

        TTF_Font *font = TTF_OpenFont("./fonts/BitcountGrid.ttf", 1000);
        if(!font){
                cerr << "Erro ao carregar a fonte" << endl;
                return 1;
        }

        const Uint32 secondary_window_id = SDL_GetWindowID(secondary_window);

        bool mode = false, done = false, button_pressed = false;
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
                                if (event.key.key == SDLK_S){
                                        if(mode) IMG_SavePNG(equalized_image, "./images/output/output_image.png");
                                        else IMG_SavePNG(grayscale_input_image, "./images/output/output_image.png");
                                }
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

                if(mode){
                        render_histogram(renderer, secondary_window, equalized_histogram, font, text_color);
                        SDL_BlitSurface(equalized_image, NULL, window_surface, NULL);
                } else {
                        render_histogram(renderer, secondary_window, histogram, font, text_color);
                        SDL_BlitSurface(grayscale_input_image, NULL, window_surface, NULL);
                }

                SDL_UpdateWindowSurface(main_window);
                SDL_RenderPresent(renderer);
        }

        delete equalized_histogram;

        TTF_CloseFont(font);
        TTF_Quit();

        SDL_DestroyWindow(main_window);
        SDL_DestroyRenderer(renderer);

        SDL_DestroyWindow(secondary_window);
        SDL_DestroySurface(input_image);
        SDL_DestroySurface(grayscale_input_image);
        SDL_DestroySurface(input_image24);
        
        SDL_Quit();

        return 0;
}
