#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
//dicas simoes: criar a ação bifurcação que simplesmente copia as ações seguintes em espelho
//ler o documento: fundamentos de algoritmos evolutivos antes de apresentar
#define DELAY 1
#define NUM_MAX_RAMOS 4
#define ENERGIA_INICIAL 0
#define CONST_PERDA 1
#define CONST_GANHO 1000000
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 600
#define NUM_MIN_PLANTAS 10
#define NUM_ACOES 100
#define TAXA_MUTACAO 15

//para compilar gcc planta.c -o planta -lm -lSDL2 -Wall -Wextra

typedef struct {
    int x, y;
} ponto;

typedef struct {
    int tipo;// 0 - Crescer, 1 - Brotar flor, 2 - Nascer folha e 3 - Novo ramo
    float angulo;
} Acao;

typedef struct {
	SDL_Renderer *renderer;
	SDL_Window *window;
} Tela;

typedef struct {
    Acao acoes[NUM_ACOES];
    double pontuacao;
    int tem_flor;
    float x, y;
    int r, g, b;
    int num_ramos;
} Individuo;

typedef struct {
    ponto local;
} Fonte_luminosa;

void lerLuzes(Fonte_luminosa** luzes, int* quantidade_luzes, Tela* tela, SDL_Rect** obstaculos, int quantidade_obstaculos);
void avaliaIndividuo(Individuo* individuo, SDL_Rect** obstaculos, int num_obs, Fonte_luminosa* luzes, int num_luzes);
void arrumaLuzes(Tela* tela, Fonte_luminosa* luzes, int quantidade_luzes);
void inicializaSDL(Tela* tela);
void lerInput(void);
void mutaIndividuo(Individuo* individuo);
void mostraTela(Tela* tela);
int estaIluminada(Fonte_luminosa luz, SDL_Rect** obstaculos, int num_obs, int x, int y);
void arrumaTela(Tela* tela);
int checaColisao(SDL_Rect rect1, SDL_Rect rect2);
int dentroObstaculo(int x, int y, SDL_Rect* obstaculo);
void lerObstaculos(SDL_Rect*** obstaculos, int* quantidade_obstaculos, Tela* tela);
Individuo* individuoAleatorio(int acoes);
void copiaIndividuo(Individuo* destino, Individuo* origem);
void arrumaObstaculos(Tela* tela, SDL_Rect** obstaculos, int quantidade_obstaculos);
void desenhaFlor(Tela* tela, float x, float y, int r, int g, int b);
void desenhaFolha(Tela* tela, float x, float y);
void desenhaCirculo(Tela* tela, int X, int Y, int raio);
void reproduz(Individuo*** populacao, int* num_plantas); 
SDL_bool colisaoRetaRect(int x1, int y1, int x2, int y2, SDL_Rect* rect);

//Projeto da Disciplina de Sistemas Evolutivos aplicados à Robótica - ICMC - 2023/2
//Grupo - Fototropismo - Lázaro Vinaud, Thales Sena e Wesley

/*
O projeto pretende simular o crescimento de plantas em direção à fontes luminosas evitando obstáculos
Esse é um fenômeno real dito por fototropismo. As plantas podem tentar ser banhadas
pelo maior número de fontes. Plantas que desenvolverem flores podem se reproduzir.

Cada indivíduo é um vetor de ações. Cada indivíduo tem uma energia, inicialmente ?ENERGIA_INICIAL?
Algumas ações são possíveis:
1 - Crescer - mecânica similar ao braço mecânico, onde se randomiza um angulo para aquela junta se esticar.
Há um custo para isso.
2 - Brotar flor - Há um gasto de energia para fazer isso e permite reprodução.
4 - Reproduzir(caso tenha flor) - Busca outra planta para reproduzir por torneio de dois,
caso não haja outra planta, o processo fracassa. A semente gerada dá pontos ao individuo.
3 - Nascer folha - Cria-se uma folha a partir de uma junta. A folha permite receber energia da luz
5 - Nascer novo ramo? - busca-se uma junta para criar um novo angulo que se tornará 
uma nova ramificação da planta 
Em resumo, o individuo começa com ENERGIA_INICIAL pontos de energia e haverá um custo para se manter
e um ganho a cada folha em contato com a luz. Esse gasto será calculado por uma função que vai 
levar em conta a quantidade de braços. O ganho também vai ser calculado por uma função que leva
em conta a quantidade de folhas na luz e a proximidade com as fontes de luz.
As fontes de luz serão definidas pelo usuário e sua intensidade também.
O usuário também definirá obstáculos, dos quais a planta deve se afastar.
Cada individuo tem uma cor para sua flor para fins de diferenciação.
A simulação tem um número min de individuos vivos, se menos do que isso existirem, o sistema gera um 
novo aleatório e substitui. Se a energia acabar ou passar o tempo de vida da planta, ela morre.
A pontuação de uma planta depende de quanto de energia ela tem e quantas sementes ela gerou.
*/

int min(int a, int b) {return (a < b) ? a : b;}
int max(int a, int b) {return (a > b) ? a : b;}

int dentroObstaculo(int x, int y, SDL_Rect* obstaculo) {
    if(x >= obstaculo->x && x <= (obstaculo->x + obstaculo->w) && y >= obstaculo->y && y <= (obstaculo->y + obstaculo->h)) return 1;
    return 0;
}

void mutaIndividuo(Individuo* individuo) {
    for(int i = 0; i < TAXA_MUTACAO; i++) {
        int qual_acao = rand()%NUM_ACOES;
        individuo->acoes[qual_acao].tipo = rand() % 4;
        individuo->acoes[qual_acao].angulo = ((float)(rand() % 6283184))/(1000000.0);
    }
    individuo->num_ramos = 1;
    individuo->x = rand() % 601 + 200;
    individuo->r = rand()%256;
    individuo->g = rand()%256;
    individuo->b = rand()%256;
}

int estaIluminada(Fonte_luminosa luz, SDL_Rect** obstaculos, int num_obs, int x, int y)
{   
    for(int i = 0; i < num_obs; i++) {
        if(colisaoRetaRect(luz.local.x, luz.local.y, x, y, obstaculos[i])) return 0;
    }
    return 1;
}

void printa_acao (Acao acao) {
    printf("A ação tem %f de angulo, tipo: %d\n", acao.angulo, acao.tipo);
}

void printa_individuo (Individuo* individuo) {
    printf("O indivíduo tem R: %d G: %d B: %d. Inicia em (%f, %f), temflor = %d e tem %lf de pontuacao\n", individuo->r, individuo->g, individuo->b, individuo->x, individuo->y, individuo->tem_flor, individuo->pontuacao);
    for(int i = 0; i < NUM_ACOES; i++) {
        printa_acao(individuo->acoes[i]);
    }
}

void copiaIndividuo(Individuo* destino, Individuo* origem) {
    destino->r = origem->r;
    destino->g = origem->g;
    destino->b = origem->b; 
    destino->tem_flor = origem->tem_flor;
    destino->pontuacao = origem->pontuacao;
    destino->num_ramos = 1;
    destino->x = origem->x;
    destino->y = origem->y;
    for(int i = 0; i < NUM_ACOES; i++) {
        destino->acoes[i] = origem->acoes[i];
    }
}

void reproduz(Individuo*** populacao, int* num_plantas) {

    int num_flores = 0;
    for(int i = 0; i < *num_plantas; i++) {
        if((*populacao)[i]->tem_flor) num_flores++;
    }
    if(num_flores >= 4) {
        int p1,p2,m1,m2;
        do {
            p1 = rand() % *num_plantas;
        } while((*populacao)[p1]->tem_flor == 0);
        do {
            p2 = rand() % *num_plantas;
        } while((*populacao)[p2]->tem_flor == 0);
        do {
            m1 = rand() % *num_plantas;
        } while((*populacao)[m1]->tem_flor == 0);
        do {
            m2 = rand() % *num_plantas;
        } while((*populacao)[m2]->tem_flor == 0);
        int p, m;
        if((*populacao)[p1]->pontuacao > (*populacao)[p2]->pontuacao) {
            p = p1;
        } else {
            p = p2;
        }
        if((*populacao)[m1]->pontuacao > (*populacao)[m2]->pontuacao) {
            m = m1;
        } else {
            m = m2;
        }
        Individuo* filho = (Individuo*) malloc(sizeof(Individuo));
        *num_plantas += 1;
        for(int i = 0; i < NUM_ACOES; i++) {
            if(rand()%2==0) {
                filho->acoes[i] = (*populacao)[p]->acoes[i];
            } else {
                filho->acoes[i] = (*populacao)[m]->acoes[i];
            }
        }
        filho->pontuacao = 0;
        filho->num_ramos = 1;
        filho->y = 560;
        filho->x = ((*populacao)[m]->x-10) + rand() % 20;
        filho->tem_flor = 0;
        if(rand()%2==0) filho->r=(*populacao)[p]->r;
        else  filho->r=(*populacao)[m]->r;
        if(rand()%2==0) filho->g=(*populacao)[p]->g;
        else  filho->g=(*populacao)[m]->g;
        if(rand()%2==0) filho->b=(*populacao)[p]->b;
        else  filho->b=(*populacao)[m]->b;
        (*populacao) = (Individuo**) realloc((*populacao), sizeof(Individuo*) * (*num_plantas));
        (*populacao)[(*num_plantas)-1] = filho;
    }

}

int checaColisao(SDL_Rect rect1, SDL_Rect rect2) 
{
    if (rect1.y + rect1.h <= rect2.y || rect1.y >= rect2.y + rect2.h ||
        rect1.x + rect1.w <= rect2.x || rect1.x >= rect2.x + rect2.w) {
        return 0;
    }

    return 1;
}

SDL_bool colisaoRetaRect(int x1, int y1, int x2, int y2, SDL_Rect* rect) {
    SDL_Point ponto1 = {x1, y1}, ponto2 = {x2, y2};
    if (SDL_EnclosePoints(&ponto1, 1, rect, NULL) || SDL_EnclosePoints(&ponto2, 1, rect, NULL)) {
        return SDL_TRUE;
    }

    int rectLeft = rect->x;
    int rectRight = rect->x + rect->w;
    int rectTop = rect->y;
    int rectBottom = rect->y + rect->h;

    if ((x1 <= rectLeft && x2 >= rectLeft) || (x2 <= rectLeft && x1 >= rectLeft)) {
        float m = (float)(y2 - y1) / (float)(x2 - x1);
        float intersectionY = m * (rectLeft - x1) + y1;
        if (intersectionY >= rectTop && intersectionY <= rectBottom) {
            return SDL_TRUE;
        }
    }

    if ((x1 <= rectRight && x2 >= rectRight) || (x2 <= rectRight && x1 >= rectRight)) {
        float m = (float)(y2 - y1) / (float)(x2 - x1);
        float intersectionY = m * (rectRight - x1) + y1;
        if (intersectionY >= rectTop && intersectionY <= rectBottom) {
            return SDL_TRUE;
        }
    }

    if ((y1 <= rectTop && y2 >= rectTop) || (y2 <= rectTop && y1 >= rectTop)) {
        float m = (float)(x2 - x1) / (float)(y2 - y1);
        float intersectionX = m * (rectTop - y1) + x1;
        if (intersectionX >= rectLeft && intersectionX <= rectRight) {
            return SDL_TRUE;
        }
    }

    if ((y1 <= rectBottom && y2 >= rectBottom) || (y2 <= rectBottom && y1 >= rectBottom)) {
        float m = (float)(x2 - x1) / (float)(y2 - y1);
        float intersectionX = m * (rectBottom - y1) + x1;
        if (intersectionX >= rectLeft && intersectionX <= rectRight) {
            return SDL_TRUE;
        }
    }

    return SDL_FALSE;
}

float distancia(float x1, float y1, float x2, float y2) {
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

void desenhaFlor(Tela* tela, float x, float y, int r, int g, int b) {
    SDL_SetRenderDrawColor(tela->renderer, r, g, b, 255);
    desenhaCirculo(tela, x, y, 2);
}

void desenhaFolha(Tela* tela, float x, float y) {
    SDL_SetRenderDrawColor(tela->renderer, 0, 200, 10, 255);
    desenhaCirculo(tela, x, y, 4);
}

void avaliaIndividuo(Individuo* individuo, SDL_Rect** obstaculos, int num_obs, Fonte_luminosa* luzes, int num_luzes) 
{   
    float* x_atuais = (float*) malloc(sizeof(float)); x_atuais[0] = individuo->x;
    float y_atual = individuo->y;
    individuo->num_ramos = 1;
    float energia_potencial = 0;
    for(int i = 0; i < NUM_ACOES; i++) {
        switch (individuo->acoes[i].tipo){
            case 0:
                y_atual += sin(individuo->acoes[i].angulo) * 20;
                for(int h = 0; h < individuo->num_ramos; h++) {
                    energia_potencial -= CONST_PERDA;
                    if((h % 2) == 0)
                        x_atuais[h] -= cos(individuo->acoes[i].angulo) * 20;
                    else
                        x_atuais[h] += cos(individuo->acoes[i].angulo) * 20;
                    for(int m = 0; m < num_obs; m++) {
                        if(dentroObstaculo(x_atuais[h], y_atual, obstaculos[m])) {
                            individuo->pontuacao = -999999;
                            return;
                        }
                    }
                }
                break;
            case 1:
                individuo->tem_flor = 1;
                energia_potencial -= 10 * CONST_PERDA;
                break;
            case 2: 
                for(int j = 0; j < num_luzes; j++)
                    for(int h = 0; h < individuo->num_ramos; h++)
                        if(estaIluminada(luzes[j], obstaculos, num_obs, x_atuais[h], y_atual))
                            energia_potencial += CONST_GANHO / (distancia(x_atuais[h], y_atual, luzes[j].local.x, luzes[j].local.y));
                break;
            case 3:
            if(individuo->num_ramos > NUM_MAX_RAMOS) break;
                individuo->num_ramos = individuo->num_ramos * 2;
                x_atuais = (float*) realloc(x_atuais, individuo->num_ramos * sizeof(float));
                for(int h = (individuo->num_ramos/2)-1; h >= 0; h--) {
                    x_atuais[h*2] = x_atuais[h];
                    x_atuais[h*2 + 1] = x_atuais[h];
                }
                break;
            default:
                break;
        }
    }
    individuo->pontuacao = energia_potencial;
}

void arrumaIndividuo(Individuo* individuo, Tela* tela)
{   
    float* x_atuais = (float*) malloc(sizeof(float)); x_atuais[0] = individuo->x;
    float y_atual = individuo->y;
    individuo->num_ramos = 1;
    for(int i = 0; i < NUM_ACOES; i++)
    {
        switch (individuo->acoes[i].tipo)
        {
        case 0:
            SDL_SetRenderDrawColor(tela->renderer, 0, 155, 0, 255);
            for(int h = 0; h < individuo->num_ramos; h++) {
                if((h%2) == 0) {
                    SDL_RenderDrawLine(tela->renderer, x_atuais[h], y_atual, x_atuais[h] + (cos(individuo->acoes[i].angulo) * 20), y_atual + (sin(individuo->acoes[i].angulo) * 20));
                    x_atuais[h] += (cos(individuo->acoes[i].angulo) * 20);
                } else {
                    SDL_RenderDrawLine(tela->renderer, x_atuais[h], y_atual, x_atuais[h] - (cos(individuo->acoes[i].angulo) * 20), y_atual + (sin(individuo->acoes[i].angulo) * 20));
                    x_atuais[h] -= (cos(individuo->acoes[i].angulo) * 20);
                }
            }
            y_atual += (sin(individuo->acoes[i].angulo) * 20);
            break;
        case 1:
            for(int h = 0; h < individuo->num_ramos; h++)
                desenhaFolha(tela, x_atuais[h], y_atual);
            break;
        case 2:
            for(int h = 0; h < individuo->num_ramos; h++)
                desenhaFlor(tela, x_atuais[h], y_atual, individuo->r, individuo->g, individuo->b);
            break;
        case 3:
        if(individuo->num_ramos > NUM_MAX_RAMOS) break;
            individuo->num_ramos = individuo->num_ramos * 2;
            x_atuais = (float*) realloc(x_atuais, individuo->num_ramos * sizeof(float));
            for(int h = (individuo->num_ramos/2)-1; h >= 0; h--) {
                x_atuais[h*2] = x_atuais[h];
                x_atuais[h*2 + 1] = x_atuais[h];
            }
            break;
        default:
            break;
        }
    }
}

Individuo* individuoAleatorio(int acoes) 
{
    Individuo* novo = (Individuo*) malloc(sizeof(Individuo));
    if(novo == NULL) exit(1);
    novo->pontuacao = 0;
    novo->tem_flor = 0;
    novo->num_ramos = 1;
    novo->y = 560;
    novo->x = (rand() % 601) + 200;
    novo->r = rand() % 256;
    novo->g = rand() % 256;
    novo->b = rand() % 256;
    for(int i = 0; i < acoes; i++) {
        novo->acoes[i].angulo = ((float)(rand() % 6283184))/(1000000);
        novo->acoes[i].tipo = rand() % 4;
    }
    return novo;
}

void lerInput(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;

			default:
				break;
		}
	}
}

void inicializaSDL(Tela* tela)
{
    srand((unsigned)time(NULL));
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		exit(1);
	}

	tela->window = SDL_CreateWindow("Fototropismo - Artificial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

	if (!tela->window)
	{
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	tela->renderer = SDL_CreateRenderer(tela->window, -1, SDL_RENDERER_ACCELERATED);

	if (!tela->renderer)
	{
		exit(1);
	}
}

void lerObstaculos(SDL_Rect*** obstaculos, int* quantidade_obstaculos, Tela* tela)
{
    SDL_Rect borda_esq = {180, 530, 20, 70};
    SDL_Rect borda_dir = {800, 530, 20, 70};
    SDL_Rect terra = {200, 560, 600, 40};
    SDL_Event event;
    int pronto = 0;
    ponto primeiro, segundo;
    int qual = 0;

    while(!pronto)
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
            case SDL_KEYDOWN:
                SDL_Keycode keyPressed = event.key.keysym.sym;
                if(keyPressed == SDLK_RETURN) 
                {
                    pronto = 1;
                    SDL_SetRenderDrawColor(tela->renderer, 0, 255, 0, 255);
                    SDL_RenderClear(tela->renderer);
                    mostraTela(tela);
                    SDL_Delay(200);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch (qual)
                {
                    case 0: 
                        primeiro.x = event.button.x;
                        primeiro.y = event.button.y;
                        qual = 1;
                        break;    
                    case 1:
                        segundo.x = event.button.x;
                        segundo.y = event.button.y;
                        SDL_Rect aux = {min(primeiro.x, segundo.x), min(primeiro.y, segundo.y), abs(primeiro.x - segundo.x), abs(primeiro.y - segundo.y)};
                        if(checaColisao(aux, borda_dir)||checaColisao(aux, borda_esq)||checaColisao(aux, terra)) break;
                        (*quantidade_obstaculos)++;
                        *obstaculos = (SDL_Rect**) realloc(*obstaculos, (*quantidade_obstaculos) * sizeof(SDL_Rect*));
                        if(*obstaculos == NULL) {printf("Erro na alocação dos obstáculos.\n"); exit(1);}
                        (*obstaculos)[*quantidade_obstaculos-1] = (SDL_Rect*) malloc(sizeof(SDL_Rect));
                        (*obstaculos)[*quantidade_obstaculos-1]->x = min(primeiro.x, segundo.x);
                        (*obstaculos)[*quantidade_obstaculos-1]->y = min(primeiro.y, segundo.y);
                        (*obstaculos)[*quantidade_obstaculos-1]->h = abs(primeiro.y - segundo.y);
                        (*obstaculos)[*quantidade_obstaculos-1]->w = abs(primeiro.x - segundo.x);
                        qual = 0;
                        break;
                    default:
                        break;
                }
                break;
			default:
                arrumaTela(tela);
                arrumaObstaculos(tela, *obstaculos, *quantidade_obstaculos);
                mostraTela(tela);
				break;
		}
	}
}

void lerLuzes(Fonte_luminosa** luzes, int* quantidade_luzes, Tela* tela, SDL_Rect** obstaculos, int quantidade_obstaculos)
{
    SDL_Event event;
    int pronto = 0;

    while(!pronto)
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
            case SDL_KEYDOWN:
                SDL_Keycode keyPressed = event.key.keysym.sym;
                if((keyPressed == SDLK_RETURN) && (*quantidade_luzes > 0)) 
                {
                    pronto = 1;
                    SDL_SetRenderDrawColor(tela->renderer, 0, 255, 0, 255);
                    SDL_RenderClear(tela->renderer);
                    mostraTela(tela);
                    SDL_Delay(200);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                int colisao = 0;
                SDL_Point aux = {event.button.x, event.button.y};
                for(int i = 0; i < quantidade_obstaculos; i++) {
                    colisao = SDL_EnclosePoints(&aux, 1, obstaculos[i], NULL); //checa se a luz que está sendo inserida colide com algum obstaculo.
                    if(colisao == SDL_TRUE) break;
                }
                if(colisao != SDL_TRUE) 
                {
                    *quantidade_luzes += 1;
                    (*luzes) = (Fonte_luminosa*) realloc(*luzes, *quantidade_luzes * sizeof(Fonte_luminosa));
                    (*luzes)[*quantidade_luzes-1].local.x = aux.x;
                    (*luzes)[*quantidade_luzes-1].local.y = aux.y;
                }
                break;
			default:
                arrumaTela(tela);
                arrumaLuzes(tela, *luzes, *quantidade_luzes);
                mostraTela(tela);
				break;
		}
	}
}

void arrumaObstaculos(Tela* tela, SDL_Rect** obstaculos, int quantidade_obstaculos) 
{
    SDL_SetRenderDrawColor(tela->renderer, 0, 0, 0, 255);
    for(int i = 0; i < quantidade_obstaculos; i++)
    {
        SDL_RenderFillRect(tela->renderer, obstaculos[i]);
    }   
}

void desenhaCirculo(Tela* tela, int X, int Y, int raio) {
    for (int y = -raio; y <= raio; ++y) {
        for (int x = -raio; x <= raio; ++x) {
            if (x * x + y * y <= raio * raio) {
                SDL_RenderDrawPoint(tela->renderer, X + x, Y + y);
            }
        }
    }
}

void arrumaLuzes(Tela* tela, Fonte_luminosa* luzes, int quantidade_luzes) 
{
    for(int i = 0; i < quantidade_luzes; i++) 
    {
        SDL_SetRenderDrawColor(tela->renderer, 255, 255, 0, 255);
        desenhaCirculo(tela, luzes[i].local.x, luzes[i].local.y, 3);
    }
}

void arrumaTela(Tela* tela)
{
	SDL_SetRenderDrawColor(tela->renderer, 92, 128, 255, 255);
	SDL_RenderClear(tela->renderer);
    SDL_SetRenderDrawColor(tela->renderer, 198, 80, 56, 255);
    SDL_Rect borda_esq = {180, 530, 20, 70};
    SDL_Rect borda_dir = {800, 530, 20, 70};
    SDL_RenderFillRect(tela->renderer, &borda_dir);
    SDL_RenderFillRect(tela->renderer, &borda_esq);
    SDL_SetRenderDrawColor(tela->renderer, 65, 32, 15, 255);
    SDL_Rect terra = {200, 560, 600, 40};
    SDL_RenderFillRect(tela->renderer, &terra);
}

void mostraTela(Tela* tela)
{
	SDL_RenderPresent(tela->renderer);
}

int main(void) {

    int quantidade_obstaculos = 0, quantidade_luzes = 0, num_plantas = NUM_MIN_PLANTAS;

    Tela tela;
    inicializaSDL(&tela);

    SDL_Rect** obstaculos = NULL;
    lerObstaculos(&obstaculos, &quantidade_obstaculos, &tela);

    Fonte_luminosa* luzes = NULL;
    lerLuzes(&luzes, &quantidade_luzes, &tela, obstaculos, quantidade_obstaculos);

    Individuo** populacao = (Individuo**) malloc(NUM_MIN_PLANTAS * sizeof(Individuo*));

    for(int i = 0; i < NUM_MIN_PLANTAS; i++) 
    {
        populacao[i] = individuoAleatorio(NUM_ACOES);
    }

    Individuo* melhor_de_todos = (Individuo*) malloc(sizeof(Individuo));
    copiaIndividuo(melhor_de_todos, populacao[0]);

    while(1) {
        lerInput();
        for(int i = 0; i < num_plantas; i++) {
            avaliaIndividuo(populacao[i], obstaculos, quantidade_obstaculos, luzes, quantidade_luzes);
        }
        for(int i = 0; i < num_plantas; i++) {
            if(populacao[i]->pontuacao > melhor_de_todos->pontuacao) {
                copiaIndividuo(melhor_de_todos, populacao[i]);
            }
        }
        //printa_individuo(melhor_de_todos);
        //printf("A pontuação do melhor de todos foi: %lf\n", melhor_de_todos->pontuacao);
        arrumaTela(&tela);
        arrumaObstaculos(&tela, obstaculos, quantidade_obstaculos);
        arrumaLuzes(&tela, luzes, quantidade_luzes);
        for(int i = 0; i < num_plantas; i++)
        {
            arrumaIndividuo(populacao[i], &tela);
        }
        int pior = 0;
        for(int i = 0; i < num_plantas; i++) {
            if(populacao[i]->pontuacao < populacao[pior]->pontuacao) {
                pior = i;
            }
        }
        copiaIndividuo(populacao[pior], melhor_de_todos);
        mutaIndividuo(populacao[pior]);
        if(rand() % 100 == 0)
            reproduz(&populacao, &num_plantas);
        mostraTela(&tela);
        SDL_Delay(DELAY);
    }

}