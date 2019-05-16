# Pong Hau K'i

Um jogo chinês chamado Pong Hau K'i, modelado no Blender e desenvolvido em C++ com a OpenGL!

Este projeto é um trabalho do curso de Engenharia de Computação da Unoesc Chapecó.

## Comandos

Confira, a seguir, os comandos para jogar:

* **Numpad 0**: Posiciona a câmera para uma visão ampla.
* **Numpad 1**: Visão frontal.
* **Numpad 3**: Visão traseira.
* **Numpad 7**: Visão superior.
* **E**: Alterna a exibição do ambiente de fundo. Quando estiver sendo exibido, o *Ambient Color* do shader é definido para **\[0.6, 0.6, 0.6\]**. Caso contrário, é definido para **\[0.1, 0.1, 0.1\]**.
* **O**: Define a projeção da câmera para perspectiva.
* **P**: Define a projeção da câmera para ortogonal.
* **R**: Mostra uma animação sobre o tabuleiro ou posiciona uma câmera estática.
* **S**: Alterna a exibição da janela de configurações. A janela também pode ser exibida ou ocultada a partir do botão nativo da *[AntTweakBar](http://anttweakbar.sourceforge.net/doc/)*.

## Configurações

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