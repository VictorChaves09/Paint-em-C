#include <stdio.h>
#include <GL/glut.h>
#include <windows.h>
#include <stdbool.h>
#include <math.h>
#define MAX_POINTS 1000
#define MAX_LINES 1000
#define MAX_POLYGONS 1000
#define DOUBLE_CLICK_INTERVAL 250
int escolha, reta, poli, lastClickTime, pontoSelec, retaSelec, poliSelec, moveX, moveY, idDelete;
float rotate, scale;
FILE* arquivo;

float xr[2], yr[2], xp[10], yp[10];
bool doubleClickDetected = false;

typedef struct{
    float x;
    float y;
}Ponto;

typedef struct{
    Ponto inicio;
    Ponto fim;
}Reta;

typedef struct{
    Ponto pontos[10];
    int vertices;
}Poligono;

typedef struct{
    Ponto pontos[MAX_POINTS];
    Reta retas[MAX_LINES];
    Poligono poligonos[MAX_POLYGONS];
}Dados;

int quantidade_pontos = 0;
Ponto pontos[MAX_POINTS];

int quantidade_retas = 0;
Reta retas[MAX_LINES];

int quantidade_poligonos = 0;
Poligono poligonos[MAX_POLYGONS];

Ponto* selePonto;
Reta* seleReta;
Poligono* selePoligono;
Dados dados;

void multiplicarMatrizes(int linhasA, int colunasA, float matrizA[linhasA][colunasA], int linhasB, int colunasB, float matrizB[linhasB][colunasB], float resultado[linhasA][colunasB]) {
    for (int i = 0; i < linhasA; i++) {
        for (int j = 0; j < colunasB; j++) {
            resultado[i][j] = 0;
            for (int k = 0; k < colunasA; k++) {
                resultado[i][j] += matrizA[i][k] * matrizB[k][j];
            }
        }
    }
}

void movePonto(Ponto* ponto, float nx, float ny){
    ponto->x += nx;
    ponto->y += ny;
}

void rotatePonto(Ponto* ponto, float angle){
    float matrizA[2][2] = {cos(angle), -sin(angle), sin(angle), cos(angle)};

    float x = (matrizA[0][0] * ponto->x) + (matrizA[0][1] * ponto->y);
    float y = (matrizA[1][0] * ponto->x) + (matrizA[1][1] * ponto->y);
    ponto->x = x;
    ponto->y = y;
}

void moveReta(Reta* reta, float nx, float ny){
    reta->inicio.x += nx;
    reta->inicio.y += ny;
    reta->fim.x += nx;
    reta->fim.y += ny;
}

void rotateReta(Reta* reta, float angle){
    float matrizA[3][3] = {cos(angle), -sin(angle), 0, sin(angle), cos(angle), 0, 0, 0, 1};
    float matrizB[3][3] = {1, 0, ((reta->inicio.x + reta->fim.x) / 2), 0, 1, ((reta->inicio.y + reta->fim.y) / 2), 0, 0, 1};
    float matrizC[3][3] = {1, 0, -((reta->inicio.x + reta->fim.x) / 2), 0, 1, -((reta->inicio.y + reta->fim.y) / 2), 0, 0, 1};
    float matrizD[3][1] = {reta->inicio.x, reta->inicio.y, 1};
    float matrizE[3][1] = {reta->fim.x, reta->fim.y, 1};
    float matrizInter[3][3];
    float matrizInter2[3][3];
    float resultado1[3][1];
    float resultado2[3][1];

    multiplicarMatrizes(3, 3, matrizB, 3, 3, matrizA, matrizInter);
    multiplicarMatrizes(3, 3, matrizInter, 3, 3, matrizC, matrizInter2);
    multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizD, resultado1);
    multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizE, resultado2);

    reta->inicio.x = resultado1[0][0];
    reta->inicio.y = resultado1[1][0];
    reta->fim.x = resultado2[0][0];
    reta->fim.y = resultado2[1][0];
}

void escalarReta(Reta* reta, float scale){
    float matrizA[3][3] = {scale, 0, 0, 0, scale, 0, 0, 0, 1};
    float matrizB[3][3] = {1, 0, ((reta->inicio.x + reta->fim.x) / 2), 0, 1, ((reta->inicio.y + reta->fim.y) / 2), 0, 0, 1};
    float matrizC[3][3] = {1, 0, -((reta->inicio.x + reta->fim.x) / 2), 0, 1, -((reta->inicio.y + reta->fim.y) / 2), 0, 0, 1};
    float matrizD[3][1] = {reta->inicio.x, reta->inicio.y, 1};
    float matrizE[3][1] = {reta->fim.x, reta->fim.y, 1};
    float matrizInter[3][3];
    float matrizInter2[3][3];
    float resultado1[3][1];
    float resultado2[3][1];

    multiplicarMatrizes(3, 3, matrizB, 3, 3, matrizA, matrizInter);
    multiplicarMatrizes(3, 3, matrizInter, 3, 3, matrizC, matrizInter2);
    multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizD, resultado1);
    multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizE, resultado2);

    reta->inicio.x = resultado1[0][0];
    reta->inicio.y = resultado1[1][0];
    reta->fim.x = resultado2[0][0];
    reta->fim.y = resultado2[1][0];
}

void movePoligono(Poligono* poligono, float nx, float ny){
    for (int i = 0; i < poligono->vertices; i++){
        poligono->pontos[i].x += nx;
        poligono->pontos[i].y += ny;
    }
}

void rotatePoligono(Poligono* poligono, float angle){
    float centerX = 0, centerY = 0;
    for (int i = 0; i < poligono->vertices; i++){
        centerX += poligono->pontos[i].x;
        centerY += poligono->pontos[i].y;
    }
    centerX = centerX / poligono->vertices;
    centerY = centerY / poligono->vertices;

    float matrizA[3][3] = {cos(angle), -sin(angle), 0, sin(angle), cos(angle), 0, 0, 0, 1};
    float matrizB[3][3] = {1, 0, centerX, 0, 1, centerY, 0, 0, 1};
    float matrizC[3][3] = {1, 0, -centerX, 0, 1, -centerY, 0, 0, 1};
    float matrizInter[3][3];
    float matrizInter2[3][3];

    multiplicarMatrizes(3, 3, matrizB, 3, 3, matrizA, matrizInter);
    multiplicarMatrizes(3, 3, matrizInter, 3, 3, matrizC, matrizInter2);

    for (int i = 0; i < poligono->vertices; i++){
        float matrizD[3][1] = {poligono->pontos[i].x, poligono->pontos[i].y, 1};
        float resultado[3][1];

        multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizD, resultado);

        poligono->pontos[i].x = resultado[0][0];
        poligono->pontos[i].y = resultado[1][0];
    }
}

void escalarPoligono(Poligono* poligono, float scale){
    float centerX = 0, centerY = 0;
    for (int i = 0; i < poligono->vertices; i++){
        centerX += poligono->pontos[i].x;
        centerY += poligono->pontos[i].y;
    }
    centerX = centerX / poligono->vertices;
    centerY = centerY / poligono->vertices;

    float matrizA[3][3] = {scale, 0, 0, 0, scale, 0, 0, 0, 1};
    float matrizB[3][3] = {1, 0, centerX, 0, 1, centerY, 0, 0, 1};
    float matrizC[3][3] = {1, 0, -centerX, 0, 1, -centerY, 0, 0, 1};
    float matrizInter[3][3];
    float matrizInter2[3][3];

    multiplicarMatrizes(3, 3, matrizB, 3, 3, matrizA, matrizInter);
    multiplicarMatrizes(3, 3, matrizInter, 3, 3, matrizC, matrizInter2);

    for (int i = 0; i < poligono->vertices; i++){
        float matrizD[3][1] = {poligono->pontos[i].x, poligono->pontos[i].y, 1};
        float resultado[3][1];

        multiplicarMatrizes(3, 3, matrizInter2, 3, 1, matrizD, resultado);

        poligono->pontos[i].x = resultado[0][0];
        poligono->pontos[i].y = resultado[1][0];
    }
}

int pickPonto(float px, float py, float mx, float my, int t){
    if (mx <= px + t && mx >= px - t && my <= py + t && my >= py - t){
        return 1;
    }
    return 0;
}

int booleanReta(int rxiE, int rxfE, int rxiD, int rxfD, int ryiAb, int ryfAb, int ryiAc, int ryfAc) {
    return (rxiE && rxfE) || (rxiD && rxfD) || (ryiAb && ryfAb) || (ryiAc && ryfAc);
}

int pickReta(float rxi, float ryi, float rxf, float ryf, float mx, float my, float xmax, float xmin, float ymax, float ymin){
    int rxiE = (rxi < (mx - xmin));
    int ryiAb = (ryi < (my - ymin));
    int rxiD = (rxi > (mx + xmax));
    int ryiAc = (ryi > (my + ymax));
    int rxfE = (rxf < (mx - xmin));
    int ryfAb = (ryf < (my - ymin));
    int rxfD = (rxf > (mx + xmax));
    int ryfAc = (ryf > (my + ymax));

    int teste = booleanReta(rxiE, rxfE, rxiD, rxfD, ryiAb, ryfAb, ryiAc, ryfAc);

    float nrxi, nryi, nrxf, nryf;

    if (teste){
        return 1;
    }else if(ryiAc){
        nryi = (my + ymax);
        nrxi = rxi + (rxf - rxi) * ((my + ymax) - ryi) / (ryf - ryi);

        if(pickReta(nrxi, nryi, rxf, ryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if(ryiAb){
        nryi = (my - ymin);
        nrxi = rxi + (rxf - rxi) * ((my - ymin) - ryi) / (ryf - ryi);

        if(pickReta(nrxi, nryi, rxf, ryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if(rxiD){
        nrxi = (mx + xmax);
        nryi = ryi + (ryf - ryi) * ((mx + xmax) - rxi) / (rxf - rxi);

        if(pickReta(nrxi, nryi, rxf, ryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if (rxiE){
        nrxi = (mx - xmin);
        nryi = ryi + (ryf - ryi) * ((mx - xmin) - rxf) / (rxi - rxf);

        if(pickReta(nrxi, nryi, rxf, ryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } if(ryfAc){
        nryf = (my + ymax);
        nrxf = rxi + (rxf - rxi) * ((my + ymax) - ryi) / (ryf - ryi);

        if(pickReta(rxi, ryi, nrxf, nryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if(ryfAb){
        nryf = (my - ymin);
        nrxf = rxi + (rxf - rxi) * ((my - ymin) - ryi) / (ryf - ryi);

        if(pickReta(rxi, ryi, nrxf, nryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if(rxfD){
        nrxf = (mx + xmax);
        nryf = ryi + (ryf - ryi) * ((mx + xmax) - rxi) / (rxf - rxi);

        if(pickReta(rxi, ryi, nrxf, nryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    } else if (rxfE){
        nrxf = (mx - xmin);
        nryf = ryi + (ryf - ryi) * ((mx - xmin) - rxf) / (rxi - rxf);

        if(pickReta(rxi, ryi, nrxf, nryf, mx, my, xmax, xmin, ymax, ymin)){
            return 1;
        }
    }

    return 0;
}

int pickPoligono(Poligono poligono, int mx, int my){
    int contagem = 0;

    for (int i = 0; i < poligono.vertices; i++){
        int o;
        int passou = 0;
        if (i == (poligono.vertices - 1)){
            o = 0;
        } else {
            o = i + 1;
        }

        Ponto p1 = poligono.pontos[i];
        Ponto p2 = poligono.pontos[o];

        if ((p1.y > my) && (p2.y > my)){
            //Ignora
        } else if ((p1.y < my) && (p2.y < my)){
            //Ignora
        } else if ((p1.x < mx) && (p2.x < mx)){
            //Ignora
        }else if ((p1.x > mx && p2.x > mx) && (((p1.y > my && p2.y < my) || (p1.y < my && p2.y > my)))){
            if (p1.y != p2.y){
                contagem++;
            } else if (p1.x == mx || p2.x == mx){
                if (passou == 0){
                    contagem++;
                    passou++;
                } else if (passou == 1){
                    passou = 0;
                }
            }
        } else {
            float xi = p1.x + ((my - p1.y) * ((p2.x - p1.x) / (p2.y - p1.y)));
            if (xi > mx){
                contagem++;
            }
        }
    }

    if ((contagem % 2) != 0){
        return 1;
    }

    return 0;
}

void addPonto(float x, float y){

    pontos[quantidade_pontos].x = x;
    pontos[quantidade_pontos].y = y;

    quantidade_pontos++;
}

void desenhaPontos(){
    glColor3f(1, 0, 0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < quantidade_pontos; i++){
        glVertex2f(pontos[i].x, pontos[i].y);
    }
    glEnd();
}

void addReta(float x1, float y1, float x2, float y2){

    retas[quantidade_retas].inicio.x = x1;
    retas[quantidade_retas].inicio.y = y1;

    retas[quantidade_retas].fim.x = x2;
    retas[quantidade_retas].fim.y = y2;

    quantidade_retas++;
}

void desenhaRetas(){
    glBegin(GL_LINES);
    for (int i = 0; i < quantidade_retas; i++){
        glColor3f(0, 1, 0);
        glVertex2f(retas[i].inicio.x, retas[i].inicio.y);
        glVertex2f(retas[i].fim.x, retas[i].fim.y);
    }
    glEnd();
}

void addPoligono(float* xi, float* yi, int o){
    for (int i = 0; i < o; i++){
        poligonos[quantidade_poligonos].pontos[i].x = xi[i];
        poligonos[quantidade_poligonos].pontos[i].y = yi[i];
    }
    poligonos[quantidade_poligonos].vertices = o;
    quantidade_poligonos++;
}

void desenhaPoligonos(){
    for (int i = 0; i < quantidade_poligonos; i++){
        glBegin(GL_POLYGON);
        glColor3f(0, 0, 1);
        for (int o = 0; o < poligonos[i].vertices ; o++){
            glVertex2f(poligonos[i].pontos[o].x, poligonos[i].pontos[o].y);
        }
        glEnd();
    }
}

void menu(int value) {
    switch (value) {
        case 1:
            escolha = 1;
            break;
        case 2:
            escolha = 2;
            break;
        case 3:
            escolha = 3;
            break;
        case 4:
            escolha = 4;
            break;
        case 5:
            escolha = 5;
            break;
        case 6:
            escolha = 6;
            break;
        case 7:
            escolha = 7;
            break;
        case 8:
            escolha = 8;
            break;
        default:
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

        int mainMenu = glutCreateMenu(menu);
        glutAddMenuEntry("Criar Ponto", 1);
        glutAddMenuEntry("Criar Reta", 2);
        glutAddMenuEntry("Criar Poligono", 3);
        glutAddMenuEntry("Selecionar Ponto", 4);
        glutAddMenuEntry("Selecionar Reta", 5);
        glutAddMenuEntry("Selecionar Poligono", 6);
        glutAddMenuEntry("Salvar", 7);
        glutAddMenuEntry("Carregar", 8);

        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }

    if (escolha == 1) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            reta = 0;
            poli = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            addPonto(x, glutGet(GLUT_WINDOW_HEIGHT) - y);
        }
    } else if (escolha == 2) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            poli = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            if (reta == 0){
                xr[0] = x;
                yr[0] = glutGet(GLUT_WINDOW_HEIGHT) - y;
                reta++;
            } else if (reta == 1){
                xr[1] = x;
                yr[1] = glutGet(GLUT_WINDOW_HEIGHT) - y;
                addReta(xr[0], yr[0], xr[1], yr[1]);
                reta = 0;
            } else {
                reta = 0;
            }
        }
    } else if (escolha == 3){
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            reta = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            if (currentTime - lastClickTime < DOUBLE_CLICK_INTERVAL) {
                doubleClickDetected = true;
            } else {
                doubleClickDetected = false;
            }

            lastClickTime = currentTime;

            xp[poli] = x;
            yp[poli] = glutGet(GLUT_WINDOW_HEIGHT) - y;
            poli++;
        }
        if (doubleClickDetected || poli >= 10){
            addPoligono(xp, yp, poli);
            poli = 0;
        }
    } else if (escolha == 4){
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            reta = 0;
            poli = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            for (int i = 0; i < quantidade_pontos; i++){
                int checa = pickPonto(pontos[i].x, pontos[i].y, x, (glutGet(GLUT_WINDOW_HEIGHT) - y), 10);
                if (checa == 1){
                    selePonto = &pontos[i];
                    pontoSelec = 1;
                    idDelete = i;
                }
            }
        }
    } else if (escolha == 5){
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            reta = 0;
            poli = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            for (int i = 0; i < quantidade_retas; i++){
                int checa = pickReta(retas[i].inicio.x, retas[i].inicio.y, retas[i].fim.x, retas[i].fim.y, x, (glutGet(GLUT_WINDOW_HEIGHT) - y), 10, 10, 10, 10);
                if (checa == 0){
                    seleReta = &retas[i];
                    retaSelec = 1;
                    idDelete = i;
                }
            }
        }
    } else if (escolha == 6){
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            reta = 0;
            poli = 0;
            pontoSelec = 0;
            retaSelec = 0;
            poliSelec = 0;
            idDelete = 0;

            for (int i = 0; i < quantidade_poligonos; i++){
                int checa = pickPoligono(poligonos[i], x, (glutGet(GLUT_WINDOW_HEIGHT) - y));
                if (checa == 1){
                    selePoligono = &poligonos[i];
                    poliSelec = 1;
                    idDelete = i;
                }
            }
        }
    } else if (escolha == 7){
        reta = 0;
        poli = 0;
        pontoSelec = 0;
        retaSelec = 0;
        poliSelec = 0;
        idDelete = 0;

        for (int i = 0; i < quantidade_pontos; i++){
            dados.pontos[i] = pontos[i];
        }
        for (int i = 0; i < quantidade_retas; i++){
            dados.retas[i] = retas[i];
        }
        for (int i = 0; i < quantidade_poligonos; i++){
            dados.poligonos[i] = poligonos[i];
        }

        arquivo = fopen("paint.bin", "wb");


        if (arquivo == NULL){
            fprintf(stderr, "Não foi possível abrir o arquivo.\n");
        }

        fwrite(&dados, sizeof(Dados), 1, arquivo);

        fclose(arquivo);
    } else if (escolha == 8){
        reta = 0;
        poli = 0;
        pontoSelec = 0;
        retaSelec = 0;
        poliSelec = 0;
        idDelete = 0;

        arquivo = fopen("paint.bin", "rb");

        printf("io");

        if (arquivo == NULL){
            fprintf(stderr, "Não foi possível abrir o arquivo.\n");
        }

        fread(&dados, sizeof(Dados), 1, arquivo);

        fclose(arquivo);

        for (int i = 0; i < quantidade_pontos; i++){
            Ponto ponto;
            pontos[i] = ponto;
        }
        for (int i = 0; i < quantidade_retas; i++){
            Reta reta;
            retas[i] = reta;
        }
        for (int i = 0; i < quantidade_poligonos; i++){
            Poligono poligono;
            poligonos[i] = poligono;
        }

        int num_pontos = sizeof(dados.pontos) / sizeof(dados.pontos[0]);
        int num_retas = sizeof(dados.retas) / sizeof(dados.retas[0]);
        int num_poligonos = sizeof(dados.poligonos) / sizeof(dados.poligonos[0]);

        for (int i = 0; i < num_pontos; i++){
            pontos[i] = dados.pontos[i];
        }
        for (int i = 0; i < num_retas; i++){
            retas[i] = dados.retas[i];
        }
        for (int i = 0; i < num_poligonos; i++){
            poligonos[i] = dados.poligonos[i];
        }

        quantidade_pontos = num_pontos;
        quantidade_retas = num_retas;
        quantidade_poligonos = num_poligonos;
    }
}

void keyboard(int key, int x, int y){
    switch (key)
    {
    case GLUT_KEY_UP:
        moveX = 0;
        moveY = 0;
        moveY += 1;
        if (pontoSelec == 1){
            movePonto(selePonto, moveX, moveY);
        }
        if (retaSelec == 1){
            moveReta(seleReta, moveX, moveY);
        }
        if (poliSelec == 1){
            movePoligono(selePoligono, moveX, moveY);
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_DOWN:
        moveX = 0;
        moveY = 0;
        moveY -= 1;
        if (pontoSelec == 1){
            movePonto(selePonto, moveX, moveY);
        }
        if (retaSelec == 1){
            moveReta(seleReta, moveX, moveY);
        }
        if (poliSelec == 1){
            movePoligono(selePoligono, moveX, moveY);
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_LEFT:
        moveY = 0;
        moveX = 0;
        moveX -= 1;
        if (pontoSelec == 1){
            movePonto(selePonto, moveX, moveY);
        }
        if (retaSelec == 1){
            moveReta(seleReta, moveX, moveY);
        }
        if (poliSelec == 1){
            movePoligono(selePoligono, moveX, moveY);
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_RIGHT:
        moveY = 0;
        moveX = 0;
        moveX += 1;
        if (pontoSelec == 1){
            movePonto(selePonto, moveX, moveY);
        }
        if (retaSelec == 1){
            moveReta(seleReta, moveX, moveY);
        }
        if (poliSelec == 1){
            movePoligono(selePoligono, moveX, moveY);
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_HOME:
        rotate = 0;
        rotate -= 0.01;
        if (pontoSelec == 1) {
            rotatePonto(selePonto, rotate);
        }
        if (retaSelec == 1){
            rotateReta(seleReta, rotate);
        }
        if (poliSelec == 1){
            rotatePoligono(selePoligono, rotate);
        }

        glutPostRedisplay();
        break;

    case GLUT_KEY_INSERT:
        rotate = 0;
        rotate += 0.01;
        if (pontoSelec == 1) {
            rotatePonto(selePonto, rotate);
        }
        if (retaSelec == 1){
            rotateReta(seleReta, rotate);
        }
        if (poliSelec == 1){
            rotatePoligono(selePoligono, rotate);
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_PAGE_UP:
        scale = 0;
        scale += 1.01;
        if (retaSelec == 1){
            escalarReta(seleReta, scale);
        }
        if (poliSelec == 1){
            escalarPoligono(selePoligono, scale);
        }

        glutPostRedisplay();
        break;

    case GLUT_KEY_PAGE_DOWN:
        scale = 0;
        scale += 1.01;
        if (retaSelec == 1){
            escalarReta(seleReta, (1 / scale));
        }
        if (poliSelec == 1){
            escalarPoligono(selePoligono, (1 / scale));
        }
        glutPostRedisplay();
        break;

    case GLUT_KEY_END:
        if (pontoSelec){
            for (int i = idDelete; i < quantidade_pontos - 1; i++){
                pontos[i] = pontos[i + 1];
            }
            quantidade_pontos -= 1;
        }
        if (retaSelec){
            for (int i = idDelete; i < quantidade_retas - 1; i++){
                retas[i] = retas[i + 1];
            }
            quantidade_retas -= 1;
        }
        if (poliSelec){
            for (int i = idDelete; i < quantidade_poligonos - 1; i++){
                poligonos[i] = poligonos[i + 1];
            }
            quantidade_poligonos -= 1;
        }
        glutPostRedisplay();
        break;

    default:
        break;
    }
}

void display(){

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    desenhaPontos();
    desenhaRetas();
    desenhaPoligonos();

    glFlush();
}

void init(){
    glClearColor(1.0, 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, 500, 0, 500);

}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Paint Alpha 0.01");

    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutSpecialFunc(keyboard);
    glutMainLoop();
    return 0;
}
