# Projeto Embarcados

Desenvolvendo um controle remoto.

## Entrega 1

### Integrantes

- Joseph Kallas
- Leonardo Andrade

### Ideia

Temos como ideia principal, integrar um controle de joystick ao Tiktok, de modo a conseguir navegar na interface. Colocando o joystick para um dos 4 lados ele irá: curtir o vídeo; passar para um próximo vídeo; voltar para o vídeo antigo; mutar/desmutar o vídeo. Além disso, teremos um potenciômetro que irá aumentar ou diminuir o volume do vídeo com base em seu giro.

### Nome

JoyStiktok.

### Usuários 

Os possíveis usuários desse controle seriam pessoas que gostariam de uma experiência diferente ao utilizar o site.

### Software/Jogo 

O controle irá controlar o Web TikTok de Desktop.

### Jornada do usuários (3 pts)

## Jornada do usuário 1:

* Um usuário está assistindo a um vídeo no TikTok, mas o vídeo está muito alto.
* Ele pega o joystick e gira o potenciômetro para diminuir o volume do vídeo.
* Ele move o joystick para baixo para ver novos vídeo até que, sem querer, acaba passando por um vídeo que gostou, sem curtir o vídeo. Assim, ele move o joystick para cima para voltar ao vídeo anterior.
* Ele move o joystick para a direita para curtir o vídeo
* O usuário continua navegando pelo TikTok, usando o joystick para controlar a interface.

## Jornada do usuário 2:
* Um usuário está assistindo a um vídeo no TikTok e percebe que o vídeo está muito baixo.
* Ele pega o joystick e gira o potenciômetro no sentido horário para aumentar o volume do vídeo.
* Quando o vídeo acaba, ele quer passar para o próximo vídeo. Ele move o joystick para baixo para passar para o próximo vídeo.
* Entretanto, o vídeo em questão está com o áudio "estourado" e ele, imediatamente, move o joystick para a esquerda para mutar o som do vídeo.
* Ele move o joystick para baixo para passar para o próximo vídeo e move para a esquerda novamente para que o som seja desmutado.

### Comandos/ Feedbacks (2 pts)

## Comandos
#### Joystick:
- Para cima: volta ao vídeo anterior
- Para baixo: passa para o próximo vídeo
- Para esquerda: muta/desmuta o vídeo
- Para direita: curte o vídeo

#### Potenciômetro:
- Ajuste manual de volume do vídeo

## Feedbacks

Para o joystick, um feedback de toque pode ser uma vibração ao mover o joystick em uma direção. Isso pode ajudar o usuário a sentir quando o joystick é movido em uma determinada direção e indicar que uma ação foi executada. Um feedback visual pode ser uma animação tela que mostra qual ação está sendo realizada quando o joystick é movido (animação de passar de vídeo, mute/desmute e like).

Já para o potenciômetro, um feedback tátil pode ser a resistência que o usuário sente ao girar o potenciômetro, indicando que o volume está sendo alterado. Um feedback visual pode ser uma barra de volume na tela que indica o nível de volume atual do vídeo.

## In/OUT (3 pts)

Para cada vez que o joystick mutar algum vídeo (movimento para a esquerda) e ele ficar sem som, o LED verde irá apagar. Quando voltar o som, o LED verde irá acender de novo.

Para qualquer movimento do joystick, o LED da placa irá piscar por alguns segundos. Assim, indicando que recebeu algum comando do usuário.

### Design (2 pts)

<!--
Faca um esboco de como seria esse controle (vai ter uma etapa que terão que detalhar melhor isso).
-->

![image](https://user-images.githubusercontent.com/79852830/226080812-e1710b83-4bfc-4d33-988c-df9c6994a957.png)

