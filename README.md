# Image Equalizer

| Nome | RA |
| --- | --- |
| Gustavo Garabetti Munhoz | 10409258 |
| João Pedro Rodrigues Vieira | 10403595 |

## Sobre o Projeto

O seguinte trabalho consiste em uma implentação realizada em C++, com auxílio da biblioteca SDL3, de um equalizador de imagem. O projeto, em resumo, detecta a distribuição das intensidades de uma imagem em escala de cinza, convertendo-a caso não esteja neste formato, e apresenta tal configuração por meio de um histograma. Neste contexto, partir do acionamento da equalização, é feita a redistribuição das intensidades dos pixels a partir da função de distribuição acumulada, sendo possível observar seu resultado tanto na imagem quanto no histograma. 

## Dependências do Projeto

Além do suporte à linguagem C++, o seguinte projeto conta com a utilização das seguintes bibliotecas SDL:
- SDL3: [https://github.com/libsdl-org/SDL/releases](https://github.com/libsdl-org/SDL/releases);
- SDL_image: [https://github.com/libsdl-org/SDL_image/releases](https://github.com/libsdl-org/SDL_image/releases);
- SDL_ttf: [https://github.com/libsdl-org/SDL_ttf/releases](https://github.com/libsdl-org/SDL_ttf/releases);
    - Para ser utilizado, o SDL_ttf ainda necessita da instalação prévia da biblioteca freetype, que pode ser obtida neste endereço: [https://freetype.org/](https://freetype.org/).

## Como Executar

Para executar o projeto, basta executar o arquivo [makefile](./makefile). A execução sugerida é a que emprega a função `all`:

```sh
make all
```

Trata-se tão somente da compilação e execução do programa, além da remoção do binário gerado no final da primeira etapa. Se preferir, execute cada função em separado ou até cada comando manualmente.

```sh
# compilação
g++ -o teste main.cpp -g -Og -Wall -Wno-unused -lSDL3_image -lSDL3_ttf -lSDL3 -lm
```

```sh
# execução
./teste caminho/da/imagem.jpg
```

```sh
# remoção do binário, caso não o deseje mais
rm -if teste
```

Caso deseje alterar a imagem lida pelo programa, basta descriminar o caminho do arquivo de imagem em sua máquina no [makefile](./makefile), ou descrevê-la durante a execução manual do programa, como no exemplo acima.

## Sobre a Implementação

O equalizador de imagem pode ser descrito através das seguintes etapas: ingestão, renderização, equalização e salvamento.

### Ingestão da Imagem

Ao descriminar o caminho de acesso a uma imagem, o programa reliza duas verificações:
- se a imagem realmente existe:
```cpp
bool file_exists(const char* fileName) {
    return std::filesystem::exists(fileName);
}
```
- se a imagem está em um formato compatível com a função `SDL_image`, que faz a leitura da imagem:
```cpp
bool supported_format(const char* fileName) {
        // Got this list based on SDL_image supported formats
        // Basically, SDL_image checks the file for each of these extensions
        // See: https://github.com/libsdl-org/SDL_image/blob/main/src/IMG.c#L52

        SDL_IOStream *stream = SDL_IOFromFile(fileName, "rb");

        return IMG_isAVIF(stream) || IMG_isBMP(stream) // etc etc etc... ;
}
```

Em seguida, após o carregamento da imagem, é feita uma verificação se a mesma está em escala de cinza, se as cores RGB que compõem um pixel da imagem são iguais entre si (ver a função `is_grayscale_image`). Se a imagem estiver em escala de cinza, perfeito! Caso não esteja, basta percorrer novamente cada pixel da imagem, utilizando a seguinte fórmula:

```cpp
SDL_Surface* to_grayscale(SDL_Surface* src_image){
        // [...]
        for(int i = 0; i < src_image->h; i++){
                for(int j = 0; j < src_image->w; j++){
                        Uint8* src = (Uint8*)src_image->pixels + i*src_image->pitch + j*bytes_per_pixel;
                        Uint8* dst = (Uint8*)grayscale_image->pixels + i*grayscale_image->pitch + j*bytes_per_pixel;

                        // >>> Fórmula de conversão <<<
                        const Uint8 color = 0.2125*src[0] + 0.7154*src[1] + 0.0721*src[2];

                        dst[0] = dst[1] = dst[2] = color;
                }
        }

        return grayscale_image;
}
```

Assim, é garantido que a imagem em questão está em escala de cinza.

### Renderização dos Dados de Imagem

Para apresentar os dados de imagem, bem como a própria imagem, foram criadas duas janelas: a primeira, contendo a imagem em escala de cinza e a seguinta, que contém o histograma, dados a respeito da imagem e o botão com o qual é possível equalizar a imagem.

Sobre o histograma, para facilitar a sua manipulação e aproveitamento de informações pertinentes a ele, foi criado um `struct`, que é criado a partir da função `create_image_histogram`:
```cpp
typedef struct Histogram {
        SDL_FRect area;
        vector<int> values;
        int total_bits;
        int max_value;
} Histogram;
```

Sobre este aspecto, é válido mencionar que todo elemento visual, seja ele histograma ou botão, foram implementados a partir de duas funções: uma que cria ("instancia" o elemento) e outra que o renderiza (ver `create_image_histogram`, `render_histogram`, `create_button_text`, `create_button` e `render_button`), dentro do loop no qual os eventos do computador, como os gerados pelo mouse e teclado, são monitorados. A exceção reside no texto, o qual possui somente uma função que o renderiza, dada a sua simplicidade (ver função `render_text`). 

Esta separação de papéis facilita a programação dos elementos na tela, uma vez que a partir da função de renderização foi possível lidar com alterações dos valores do histograma e informações sobre a  imagem após a equalização da mesma, como também a alteração dos elementos visuais do botão ao ser submetido a alguns elementos do mouse. 

### Equalização

A equalização da imagem é feita no próprio histograma antes da própria imagem. Daí a importância de criar o histograma antes de renderizá-lo. Vejamos: a função `equalize_histogram` recebe os dados do histograma e retorna um novo, agora equalizado. O que a função faz é a aplicação função de distribuição acumulada, que utiliza um mapeamento das probabilidades de ocorrência dos bits da imagem em escala de cinza, realizado pela função `get_mapped_bits`, e rearranja as contagens de acordo com o mapeamento.

```cpp
Histogram* equalize_histogram(Histogram* src_histogram){
        // [...]

        for(int i = 0; i < (int) intensity.size(); i++){
                equalized_intensity[mapping_function[i]] += intensity[i];
                new_max_value = max(new_max_value, equalized_intensity[mapping_function[i]]);
        }

        // [...] Um novo histograma é "instanciado" e retornado
}
```

### Salvamento

Salvar a imagem desejada não representa um desafio maior do que equalizar o histograma. Na verdade, basta monitorar quando a tecla "S" é pressionada e salvar a imagem, sendo ela equalizada ou não.

```cpp
else if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_S){
                if(mode) IMG_SavePNG(equalized_image, "./images/output/output_image.png");
                else IMG_SavePNG(grayscale_input_image, "./images/output/output_image.png");
        }
}
```
