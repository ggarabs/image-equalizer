## MVP

### Escopo:

- [ ] O programa deve ser capaz de carregar imagens nos formatos mais comuns, como PNG, JPG e BMP, usando a biblioteca SDL_image.
- [ ] Deve tratar possíveis problemas, como arquivo não encontrado ou arquivo que não seja um formato de imagem válido.
- [x] O programa precisa implementar uma função que verifica se a imagem carregada já está em escala de cinza ou se é colorida.
- [x] Caso a imagem seja colorida, o programa deve converter a imagem para escala de cinza usando a seguinte fórmula: 𝑌 = 0.2125 ∗ 𝑅 + 0.7154 ∗ 𝐺 + 0.0721 ∗ 𝐵.
- [x] Janela principal: Deve exibir a imagem que está sendo processada. O tamanho da janela deve se adaptar ao tamanho da imagem carregada e deve iniciar centralizada no monitor principal.
- [ ] Janela secundária (filha da janela principal): Uma janela de tamanho fixo (definido por você), posicionada ao lado da janela principal. Deve exibir o histograma da imagem e um botão de operação.
- [x] Na janela secundária, o programa deve exibir o histograma da imagem.
- [ ] O programa deve analisar o histograma e exibir as seguintes informações:
- [x] Média de intensidade: Classificar a imagem como "clara", "média" ou "escura".
- [ ] Desvio padrão: Classificar o contraste da imagem como "alto", "médio" ou "baixo".
- [x] Na janela secundária, deve haver um botão (desenhado com primitivas da SDL).
- [ ] Ao clicar no botão, o programa deve equalizar o histograma da imagem, atualizando a imagem exibida na janela principal e o histograma na janela secundária.
- [x] O texto do botão deve mudar para refletir a ação (ex.: "Equalizado" / "Original")
- [x] O estado do botão deve refletir as ações do usuário (ex.: cor azul para estado "neutro", cor azul claro para estado "mouse em cima do botão", cor azul escuro para estado "botão clicado")
- [X] Ao pressionar a tecla S do teclado, o programa deve salvar a imagem atualmente exibida na janela principal em um arquivo chamado output_image.png.
- [X] Caso o arquivo output_image.png já exista, o programa deve sobrescrever o arquivo.
