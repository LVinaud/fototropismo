# Fototropismo evolutivo
Tentamos simular o comportamento dito por fototropismo positivo, no qual plantas se desenvolvem em direção à uma fonte luminosa.
![](https://github.com/LVinaud/fototropismo/blob/main/Positive_Phototropism_Demonstration.gif)

# Sobre o projeto
Com a apropriaçao dos metodos e conceitos aprendidos em sala, tentamos fazer com que plantas se desenvolvam em direçao a luz de maneira otimizada ao mesmo tempo que desviem de obstáculos que não permitem a passagem de luz. Utilizamos a biblioteca SDL2 para 
a parte gráfica do programa.

# O algoritmo
## Evolução
Um sistema evolutivo inspirado na evolução real busca encontrar soluções razoáveis para problemas de difícil solução por métodos convencionais.
Uma população de soluções é avaliada e recebe uma pontuação no contexto do problema, novas soluções são geradas a partir da mutação(pequena alteração) ou reprodução 
das melhores soluções encontradas até então, o processo se repete indefinidamente. Com a preservação da melhor solução já encontrada, o algorítmo sempre gerará soluções melhores ou equivalentes ao já encontrado.
## População
A população é composta de individuos completamente aleatorios, onde uma struct composta pelas seguintes variaveis:
- ```Acao acoes[NUM_ACOES]``` um vetor de structs que compõem as as açoes do indiviuo (o gene).
- ```pontuacao``` a nota que será atribuída na hora da avaliação para o indivíduo.
- ```tem_flor``` auxília para saber se o indivíduo pode realizar reprodução sexuada.
- ```x, y``` ponto de brotamento do indivíduo.
- ```r, g, b``` coloração das flores do indivíduo. 
- ```num_ramos``` quantidade de ramificações que a planta gerou.

Cada indivíduo portanto é uma sequência de ações, há 4 tipos de ações. Crescer, brotar flor, nascer folha e bifurcar. Crescer e brotar flor gastam energia, enquanto que a proximidade das folhas com fontes luminosas não obstruídas adicionam energia ao individuo. Bifurcar gera uma nova ramificação simétrica.
## Avaliação
A nota final do indivíduo será a quantidade final de energia que ele possui no final do processamento de suas ações
onde crescer gasta energia e gerar flor gasta energia sendo assim o que aumenta o valor da energia sendo as folhas isso estando relacionado com a distancia que elas estão da fonte luminosa (portanto há um incentivo para crescer próximo à luz).
## Estratégias avançadas utilizadas
### Predação + Elitismo
Consiste em escolher o pior indivíduo de toda a população e substituí-lo por uma versão mutada do melhor já encontrado.
### Torneio de 2
No torneio de 2, 4 indivíduos aleatórios(dentre os floridos) serão selecionados, dois deles comparam sua pontuação para decidir quem será o pai,
os outros dois para quem será a mãe. As ações do novo indivíduo serão sorteadas dentre as do pai e as da mãe.
No nosso programa ocorre está reprodução com indivíduos floridos, onde a criança nasce ao lado da mãe escolhida através desse método.
### Mutação Variável
Em cada iteração do código, checamos se a pontuação do melhor de todos está igual à iteração passada, se isso se repetir por 50 gerações, a taxa de mutação é acrescida de um.

# Algumas demontrações do programa
![](https://github.com/LVinaud/fototropismo/blob/main/Screencast%20from%2013-12-2023%2011%2008%2027.gif)
![](https://github.com/LVinaud/fototropismo/blob/main/Screencast%20from%2013-12-2023%2011%2006%2019.gif)


