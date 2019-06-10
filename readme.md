# Pong Hau K'i

Um jogo chinês chamado Pong Hau K'i, modelado no Blender e desenvolvido em C++ com a OpenGL!

Este projeto é um trabalho do curso de Engenharia de Computação da Unoesc Chapecó.

*Jogo em fase de desenvolvimento e aprimoramento.*

## Instruções

Primeiramente, é necessário informar os nomes dos dois jogadores, logo que o jogo é iniciado. Depois, clique em qualquer lugar para tirar o foco dos campos de inserção e validar as ações. Feito isto, clique no botão *Confirmar* que aparecerá logo abaixo dos campos.

Pressione **ENTER** para continuar, conforme é mostrado na tela.

O jogo consiste em simplesmente cada jogador mover uma peça, alternadamente, com o objetivo de bloquear os movimentos do adversário.

Para mover as peças, basta que o jogador da rodada clique em sua peça.

O Jogador 1 sempre terá as peças azuis e sempre será o primeiro a jogar em cada partida. O Jogador 2, obviamente, terá as peças verdes.

O jogador que cumprir o objetivo, antes que o contador de jogadas chegue à zero, vence a partida. Caso contrário, será considerado empate.

A cada partida finalizada, é necessário pressionar **ENTER** para iniciar a próxima, como pode ser visto no texto *Pressione ENTER para continuar*, que fica "piscando" no tabuleiro.

Em todas as partidas, durante ou quando finalizadas, podem ser observadas informações úteis na canto superior esquerdo, como o jogador da vez, o *status* da última partida, placar, informações da partida atual, teclas para executar determinadas ações durante o jogo, etc.

Finalmente, para sair do jogo, basta pressionar a tecla **ESC**.

Divirtam-se!

## Comandos

Confira, a seguir, os comandos para jogar:

* **Numpad 0**: Posiciona a câmera para uma visão ampla.
* **Numpad 1**: Visão frontal.
* **Numpad 3**: Visão traseira.
* **Numpad 7**: Visão superior.
<br><br>
* **E**: Alterna a exibição do ambiente de fundo. Quando estiver sendo exibido, o *Ambient Color* do shader é definido para (0.6, 0.6, 0.6). Caso contrário, é definido para (0.1, 0.1, 0.1).
* **O**: Define a projeção da câmera para perspectiva.
* **P**: Define a projeção da câmera para ortogonal.
* **R**: Mostra uma animação sobre o tabuleiro ou posiciona uma câmera estática.
<br><br>
* **M**: Alterna a exibição da janela de informações do jogo.
* **S**: Alterna a exibição da janela de configurações.

Ambas as janelas também podem ser exibidas ou ocultadas a partir dos botões respectivos nativos da *[AntTweakBar](http://anttweakbar.sourceforge.net/doc/)*.

## Informações do jogo

No canto superior esquerdo são exibidas algumas informações úteis do jogo:

* **Jogador da vez**: Quem está na vez de jogar.
* **Situação da última partida**: Informa como finalizou a última partida, como quem foi o vencedor ou se houve empate.
* **Placar**: Simplesmente um placar com um contador de vitórias para cada jogador e um contador de empates.
* **Jogadas restantes**: Exibe quantas jogadas ainda podem ser realizada na partida atual. Cada partida tem **50** jogadas!
* **Peça selecionada**: Indica de quem é a peça que está sob o cursor do mouse.
* **Última jogada**: Exibe as casa de origem e destino da última jogada realizada.
* **Posição do cursor**: Apenas exibe as coordenadas do cursor do mouse na tela.
* **Ray-picking**: Mostra a direção do raio do *ray-picking*, apenas para demonstração. Este raio é responsável pela seleção de uma peça através do clique!
* **Teclas de ações**: Algumas instruções de ações básicas que podem ser realizadas, como exibir ou ocultar uma janela ou fechar o jogo.

## Configurações

Ao pressionar **S**, ficam disponíveis as seguintes opções:

### Gráficos

* **Ambiente de fundo**: Alterna a exibição do ambiente de fundo.

### Câmera

* **Distância focal**: Ajusta a distância focal da câmera, numa faixa de 1 à 100, com um passo de 0.01 unidade.
* **Visão**: Seleciona uma visão da câmera. As opções são *Ampla*, *Frontal*, *Traseira* e *Superior*.
* **Projeção**: Alterna a projeção da câmera entre perspectiva e ortogonal.
* **Animação**: Mostra uma animação sobre o tabuleiro ou posiciona uma câmera estática.

### Iluminação

* **Cor**: Permite alterar a cor da iluminação do jogo.
* **Brilho**: Ajusta o brilho da iluminação, numa faixa de 0 à 10000, com um um passo de 10 unidades.