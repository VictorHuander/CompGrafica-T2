// **********************************************************************
// PUCRS/Escola Politécnica
// COMPUTAÇÃO GRÁFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

// Para uso do Windows, sugere-se a versao 17 do Code::Blocks

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
// Victor Vasconcellos e Heitor Hunrath

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "ListaDeCoresRGB.h"
#include "Temporizador.h"
#include "Linha.h"

Temporizador T;
double AccumDeltaT=0;

// Variaveis que controlam o triangulo do campo de visao
Poligono PontosDoCenario, CampoDeVisao, TrianguloBase;
float AnguloDoCampoDeVisao=0.0;

// Limites logicos da area de desenho
Ponto Min, Max, Tamanho, Meio;
Ponto PosicaoDoCampoDeVisao, PontoClicado;

bool desenhaEixos = true;
bool desenhaEnvelope = false;
bool desenhaForcaBruta = false;
bool quadTreeOn = false;
bool FoiClicado = false;


Poligono envelope;
Poligono pontosASeremConsiferados;
Poligono pontosDentroDoEnvelope;
Ponto p1, p2, p3, p4;

// **********************************************************************
// GeraPontos(int qtd)
//      Método que gera pontos aleatórios no intervalo [Min..Max]
// **********************************************************************
void GeraPontos(unsigned long int qtd, Ponto Min, Ponto Max)
{
    time_t t;
    Ponto Escala;
    Escala = (Max - Min) * (1.0/1000.0);
    srand((unsigned) time(&t));
    for (int i = 0;i<qtd; i++)
    {
        float x = rand() % 1000;
        float y = rand() % 1000;
        x = x * Escala.x + Min.x;
        y = y * Escala.y + Min.y;
        PontosDoCenario.insereVertice(Ponto(x,y));
    }
}

// **********************************************************************
// void CriaTrianguloDoCampoDeVisao()
//  Cria um triangulo a partir do vetor (1,0,0), girando este vetor
//  em 45 e -45 graus.
//  Este vetor fica armazenado nas variáveis "TrianguloBase" e
//  "CampoDeVisao"
// **********************************************************************
void CriaTrianguloDoCampoDeVisao()
{
    Ponto vetor = Ponto(1,0,0);

    TrianguloBase.insereVertice(Ponto(0,0,0));
    CampoDeVisao.insereVertice(Ponto(0,0,0));

    vetor.rotacionaZ(45);
    TrianguloBase.insereVertice(vetor);
    CampoDeVisao.insereVertice(vetor);

    vetor.rotacionaZ(-90);
    TrianguloBase.insereVertice(vetor);
    CampoDeVisao.insereVertice(vetor);

}
// **********************************************************************
// void PosicionaTrianguloDoCampoDeVisao()
//  Posiciona o campo de visão na posicao PosicaoDoCampoDeVisao,
//  com a orientacao "AnguloDoCampoDeVisao".
//  O tamanho do campo de visão eh de 25% da largura da janela.
// **********************************************************************
float campoDeVisaoConstante = 0.25;

void PosicionaTrianguloDoCampoDeVisao()
{
    float tamanho = Tamanho.x * campoDeVisaoConstante;

    Ponto temp;
    for (int i=0;i<TrianguloBase.getNVertices();i++)
    {
        temp = TrianguloBase.getVertice(i);
        temp.rotacionaZ(AnguloDoCampoDeVisao);
        CampoDeVisao.alteraVertice(i, PosicaoDoCampoDeVisao + temp*tamanho);
    }
    // recalcular envelope aqui


}

void PosicionaCampoDeVisao(int n)
{
    switch (n) {
        case 1:
            AnguloDoCampoDeVisao = 0;
            PosicaoDoCampoDeVisao = Meio;
            break;
        case 2:
            AnguloDoCampoDeVisao = 90;
            PosicaoDoCampoDeVisao = Meio;
            break;
        case 3:
            AnguloDoCampoDeVisao = 90;
            PosicaoDoCampoDeVisao = Meio*0.5;
            break;
        case 4:
            AnguloDoCampoDeVisao = 0;
            PosicaoDoCampoDeVisao = Meio + Meio*0.5;
            break;
        default:
            break;
    }

    PosicionaTrianguloDoCampoDeVisao();
}
// **********************************************************************
// void AvancaCampoDeVisao(float distancia)
//  Move o campo de visão "distancia" unidades pra frente ou pra tras.
// **********************************************************************
void AvancaCampoDeVisao(float distancia)
{
    Ponto vetor = Ponto(1,0,0);
    vetor.rotacionaZ(AnguloDoCampoDeVisao);
    PosicaoDoCampoDeVisao = PosicaoDoCampoDeVisao + vetor * distancia;
}
// **********************************************************************
// void init()
//  Faz as inicializacoes das variaveis de estado da aplicacao
// **********************************************************************
void init()
{
    // Define a cor do fundo da tela (AZUL)
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    // Gera ou Carrega os pontos do cenario.
    // Note que o "aspect ratio" dos pontos deve ser o mesmo
    // da janela.

    // PontosDoCenario.LePoligono("PontosDenteDeSerra.txt");
    //GeraPontos(1000, Ponto(0,0), Ponto(500,500));

    //PontosDoCenario.LePoligono("CenarioDeTeste.txt");
    PontosDoCenario.LePoligono("CenarioGrande.txt");

    PontosDoCenario.obtemLimites(Min,Max);
    //Min.x--;Min.y--;
    //Max.x++;Max.y++;

    Meio = (Max+Min) * 0.5; // Ponto central da janela
    Tamanho = (Max-Min);  // Tamanho da janela em X,Y

    // Ajusta variaveis do triangulo que representa o campo de visao
    PosicaoDoCampoDeVisao = Meio;
    AnguloDoCampoDeVisao = 0;

    // Cria o triangulo que representa o campo de visao
    CriaTrianguloDoCampoDeVisao();
    PosicionaTrianguloDoCampoDeVisao();

}

double nFrames=0;
double TempoTotal=0;
// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0/30) // fixa a atualização da tela em 30
    {
        AccumDeltaT = 0;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        //cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        //cout << "Nros de Frames sem desenho: " << nFrames << endl;
        //cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
        TempoTotal = 0;
        nFrames = 0;
    }
}
// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
// **********************************************************************
void reshape( int w, int h )
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x,Max.x,
            Min.y,Max.y,
            0,1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
// **********************************************************************
//
// **********************************************************************
void DesenhaEixos()
{

    glBegin(GL_LINES);
    //  eixo horizontal
        glVertex2f(Min.x,Meio.y);
        glVertex2f(Max.x,Meio.y);
    //  eixo vertical
        glVertex2f(Meio.x,Min.y);
        glVertex2f(Meio.x,Max.y);
    glEnd();
}

void DesenhaLinha(Ponto P1, Ponto P2)
{
    glBegin(GL_LINES);
        glVertex3f(P1.x,P1.y,P1.z);
        glVertex3f(P2.x,P2.y,P2.z);
    glEnd();
}
// **********************************************************************
//  void display( void )
//
bool estaDentroDoEnvelope(Ponto ponto) {
    // p1 p2
    // p3 p4
    if ((ponto.x <= p2.x && ponto.x >= p3.x ) && (ponto.y <= p2.y && ponto.y >= p3.y )) {
        return true;
    }
        return false;
}

bool pontoPestaNaEsquedaDaRetaAB(Ponto p, Ponto a, Ponto b) {
    // https://algorithmtutor.com/Computational-Geometry/Check-if-a-point-is-inside-a-polygon/
    float x = p.x;
    float y = p.y;

    float x1 = a.x;
    float y1 = a.y;

    float x2 = b.x;
    float y2 = b.y;

    float A = - (y2 - y1);
    float B = x2 - x1;
    float C = -((A * x1) + (B * y1) );
    float resultado = (A * x) + (B * y) + C;
    if (resultado > 0) {
        return false; }
    return true;
}


bool estaDentroDoPoligonoConvexo(Ponto ponto, Poligono poligono) {

    unsigned long numeroDePontos = poligono.getNVertices();
    for(int n = 0; n <= numeroDePontos; n++){
        Ponto pontoAtual = poligono.getVertice(n); // checar se esta a esquerda
        Ponto proximoPonto = poligono.getVertice((n + 1) % numeroDePontos);
        if (!pontoPestaNaEsquedaDaRetaAB(ponto, pontoAtual, proximoPonto)) {
            return false;
        }
    }
    return true;
}

void DesenhaVerticesColoridos (Poligono Poly)
{
    for (int i=0; i<Poly.getNVertices(); i++)
    {
        Ponto P;
        P = Poly.getVertice(i); // obtem a coordenada
        glBegin(GL_POINTS);
//            if (P.x> P.y == 0) // critério para escolher a cor
                defineCor(NeonPink);
//            else defineCor(GreenYellow);
            glVertex3f(P.x,P.y,P.z);
        glEnd();
    }
}
// nos ultimos sldies de fundamentos matematicos está a explicação matematica da forca bruta e nas mensagens do email do moolde esta um codigo quase implementado
Poligono forcaBruta(Poligono pontosPoligono, Poligono poligonoConvexo) {

    return Poligono();
}

int countEnvelope;
void geradorDeEnvelope() {

    glLineWidth(4);
    glColor3f(0.2,0.5,0.7);
    Poligono envelope = Poligono();
    Ponto min, max;
    CampoDeVisao.obtemLimites(min, max);
    // p1 p2
    // p3 p4
    p1 = Ponto(min.x, max.y);
    p2 = Ponto(max.x, max.y);
    p3 = Ponto(min.x, min.y);
    p4 = Ponto(max.x, min.y);
    envelope.insereVertice(p1);
    envelope.insereVertice(p2);
    envelope.insereVertice(p4);
    envelope.insereVertice(p3);
    envelope.desenhaPoligono();

    unsigned long numeroDePontos = pontosASeremConsiferados.getNVertices();
    pontosDentroDoEnvelope = Poligono(); // talvez ocorra um memory leak aqui pois não estou excluindo os ontos antigos
    glPointSize(4);
    for(int n = 0; n <= numeroDePontos; n++){
        countEnvelope = n;
        Ponto pontoAtual = PontosDoCenario.getVertice(n);
        if (estaDentroDoEnvelope(pontoAtual)) {
            glBegin(GL_POINTS);
                    defineCor(NeonPink);
                glVertex3f(pontoAtual.x,pontoAtual.y,pontoAtual.z);
            glEnd();

            pontosDentroDoEnvelope.insereVertice(pontoAtual);
        }
    }
    glPointSize(1);
    pontosASeremConsiferados = pontosDentroDoEnvelope;

}

//    P2
// P3
bool pontoEstaEntreOsPontosBC(Ponto ponto, Ponto P2, Ponto P3) {
    if ((ponto.x <= P2.x && ponto.x >= P3.x ) && (ponto.y <= P2.y && ponto.y >= P3.y )) {
        return true;
    }
    return false;
}
// p1  p2
//   pm
// p3  p4
//struct RECT
//{
//    double x,y, w,h;
//
//    RECT(double a,double b,double c,double d)
//    {
//    x=a; y=b; w=c; h=d;
//    }
//};
//bool contains(RECT R1, RECT R2)
//{
//    if (   (R2.x+R2.w) < (R1.x+R1.w)
//        && (R2.x) > (R1.x)
//        && (R2.y) > (R1.y)
//        && (R2.y+R2.h) < (R1.y+R1.h)
//        )
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
//}
bool campoDeVisaoCabeNoPoligono(Poligono poligono) {
    Ponto min, max;
    poligono.obtemLimites(min, max);
//    p2 = Ponto(max.x, max.y);
//    p3 = Ponto(min.x, min.y);
    Linha linha = Linha(min.x, min.y, max.x, max.y);

    Ponto minC, maxC;
    CampoDeVisao.obtemLimites(minC, maxC);
    Linha linhaC = Linha(minC.x, minC.y, maxC.x, maxC.y);
    bool result = linha.EnvelopesColidem(linhaC);

    return result;

//    RECT rect1 = RECT(min.x, min.y, max.x - min.x,  max.y - min.y);
//    RECT rect2 = RECT(minC.x, minC.y, maxC.x - minC.x,  maxC.y - minC.y);
//    return  contains(rect1, rect2);

}

bool quadTreeRecursivo(Ponto ponto, Poligono poligono, int profundidade) {

    Ponto p1, p2, p3, p4, pm;
    Ponto min, max;
    poligono.obtemLimites(min, max);
    p1 = Ponto(min.x, max.y);
    p2 = Ponto(max.x, max.y);
    p3 = Ponto(min.x, min.y);
    p4 = Ponto(max.x, min.y);
    pm = Ponto( ( ((max.x - min.x) / 2 )+ min.x) , ( ((max.y - min.y) / 2 )  + min.y) );

    // p1  p2
    //   pm
    // p3  p4

    // p1  p12 p2
    // p13 pm  p24
    // p3  p34 p4

    Ponto p12, p13, p24, p34;
    p12 = Ponto(pm.x, p2.y);
    p13 = Ponto(p3.x, pm.y);
    p24 = Ponto(p2.x, pm.y);
    p34 = Ponto(pm.x, p3.y);

// for pontosDoPoligono
// if norte ou sul
// if lest oest

    // NE
    if(pontoEstaEntreOsPontosBC(ponto, p2, pm)) {
        Poligono novoPoligono = Poligono();
        //p12 p2
        //pm  p24
        novoPoligono.insereVertice(p12);
        novoPoligono.insereVertice(p2);
        novoPoligono.insereVertice(p24);
        novoPoligono.insereVertice(pm);
        novoPoligono.desenhaPoligono();
        if (campoDeVisaoCabeNoPoligono(novoPoligono)) {
            if (profundidade < 4 ) {
            return quadTreeRecursivo(ponto, novoPoligono, profundidade + 1);
            } else {
                return estaDentroDoPoligonoConvexo(ponto, CampoDeVisao);
            }
        }
    }


    // NW
    if(pontoEstaEntreOsPontosBC(ponto, p12, p13)) {
        Poligono novoPoligono = Poligono();
        //p1 p12
        //p13 pm
        novoPoligono.insereVertice(p1);
        novoPoligono.insereVertice(p12);
        novoPoligono.insereVertice(pm);
        novoPoligono.insereVertice(p13);
        novoPoligono.desenhaPoligono();
        if (campoDeVisaoCabeNoPoligono(novoPoligono)) {
            if (profundidade < 4 ) {
            return quadTreeRecursivo(ponto, novoPoligono, profundidade + 1);
            } else {
                return estaDentroDoPoligonoConvexo(ponto, CampoDeVisao);
            }
        }
    }


    // SE
    if(pontoEstaEntreOsPontosBC(ponto, p24, p34)) {
        Poligono novoPoligono = Poligono();
        //pm p24
        //p34 p4
        novoPoligono.insereVertice(pm);
        novoPoligono.insereVertice(p24);
        novoPoligono.insereVertice(p4);
        novoPoligono.insereVertice(p34);
        novoPoligono.desenhaPoligono();
        if (campoDeVisaoCabeNoPoligono(novoPoligono)) {
            if (profundidade < 4 ) {
            return quadTreeRecursivo(ponto, novoPoligono, profundidade + 1);
            } else {
                return estaDentroDoPoligonoConvexo(ponto, CampoDeVisao);
            }
        }
    }


    // SW
    if(pontoEstaEntreOsPontosBC(ponto, pm, p3)) {
        Poligono novoPoligono = Poligono();
        //p13 pm
        //p3 p34
        novoPoligono.insereVertice(p13);
        novoPoligono.insereVertice(pm);
        novoPoligono.insereVertice(p34);
        novoPoligono.insereVertice(p3);
        novoPoligono.desenhaPoligono();
        if (campoDeVisaoCabeNoPoligono(novoPoligono)) {
            if (profundidade < 4 ) {
            return quadTreeRecursivo(ponto, novoPoligono, profundidade + 1);
            } else {
                return estaDentroDoPoligonoConvexo(ponto, CampoDeVisao);
            }
        }
    }


    return false;
}

bool estaDentroDoPoligonoConvexoQuadTree(Ponto ponto, Poligono poligono) {
    return quadTreeRecursivo(ponto, poligono, 0);
}

float minCenario = 0;
float maxCenario = 500;

int countQuadTree;

void geradorDeQuadTree() {
    unsigned long numeroDePontos = pontosASeremConsiferados.getNVertices();

       Ponto p1, p2, p3, p4;
        p1 = Ponto(minCenario, maxCenario);
        p2 = Ponto(maxCenario, maxCenario);
        p3 = Ponto(minCenario, minCenario);
        p4 = Ponto(maxCenario, minCenario);
        Poligono campoTotal = Poligono();
        campoTotal.insereVertice(p1);
        campoTotal.insereVertice(p2);
        campoTotal.insereVertice(p4);
        campoTotal.insereVertice(p3);

    glPointSize(9);
    for(int n = 0; n <= numeroDePontos; n++){
        countQuadTree = n;
        Ponto pontoAtual = pontosASeremConsiferados.getVertice(n);
        if (estaDentroDoPoligonoConvexoQuadTree(pontoAtual, campoTotal)) {
            glBegin(GL_POINTS);
                    defineCor(LightSteelBlue);
                glVertex3f(pontoAtual.x,pontoAtual.y,pontoAtual.z);
            glEnd();
        }
    }
    glPointSize(1);

}
int countForcaBruta;

void geradorDeForcaBruta() {

    unsigned long numeroDePontos = pontosASeremConsiferados.getNVertices();


    glPointSize(7);
    for(int n = 0; n <= numeroDePontos; n++){
        Ponto pontoAtual = pontosASeremConsiferados.getVertice(n);
        countForcaBruta = n;

        if (estaDentroDoPoligonoConvexo(pontoAtual, CampoDeVisao)) {

            glBegin(GL_POINTS);
                    defineCor(LightWood);
                glVertex3f(pontoAtual.x,pontoAtual.y,pontoAtual.z);
            glEnd();
        }
    }
    glPointSize(1);
}
// **********************************************************************
void display( void )
{

    // Limpa a tela coma cor de fundo
    glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites lÛgicos da ·rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // Coloque aqui as chamadas das rotinas que desenham os objetos
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    pontosASeremConsiferados = PontosDoCenario;

    if (desenhaEixos)
    {
        glLineWidth(1);
        glColor3f(1,1,1); // R, G, B  [0..1]
        DesenhaEixos();
    }
    if (desenhaEnvelope) {
        geradorDeEnvelope();
    }
    if (desenhaForcaBruta) {
        geradorDeForcaBruta();
    }
    if (quadTreeOn) {
        geradorDeQuadTree();
    }
    //glPointSize(5);
    glColor3f(1,1,0); // R, G, B  [0..1]
    PontosDoCenario.desenhaVertices();

    glLineWidth(3);
    glColor3f(1,0,0); // R, G, B  [0..1]
    CampoDeVisao.desenhaPoligono();

    if (FoiClicado)
    {
        //PontoClicado.imprime("- Ponto no universo: ", "\n");
        FoiClicado = false;
    }

    glutSwapBuffers();
}
// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo número de segundos e informa quanto frames
// se passaram neste período.
// **********************************************************************
void ContaTempo(double tempo)
{
    Temporizador T;

    unsigned long cont = 0;
    cout << "Inicio contagem de " << tempo << "segundos ..." << flush;
    while(true)
    {
        tempo -= T.getDeltaT();
        cont++;
        if (tempo <= 0.0)
        {
            cout << "fim! - Passaram-se " << cont << " frames." << endl;
            break;
        }
    }

}
// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
//
// **********************************************************************
void campoVisaoPrint(){
    for(int i = 0; i < CampoDeVisao.getNVertices(); i++){
        cout << CampoDeVisao.getVertice(i).x;
    }
}
Ponto minimo, maximo;
void minMaxPrint(){

    CampoDeVisao.obtemLimites(minimo, maximo);
    minimo.imprime();
    cout << " "<<endl;
}
void desenhaEnvelopeOnOff() {
    if (desenhaEnvelope == true) {
        desenhaEnvelope = false;
    } else {
        desenhaEnvelope = true;
    }
}
void desenhaForcaBrutaOnOff() {
    if (desenhaForcaBruta == true) {
        desenhaForcaBruta = false;
    } else {
        desenhaForcaBruta = true;
    }
}
void quadTreeOnOff() {
    if (quadTreeOn == true) {
        quadTreeOn = false;
    } else {
        quadTreeOn = true;
    }
}

void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;
        case '1':
        case '2':
        case '3':
        case '4':
            int i;
            i = key - '0';
            PosicionaCampoDeVisao(i);
            cout<<"posicao " << i<<endl;
            break;
        case 't':
            ContaTempo(3);
            break;
        case ' ':
            desenhaEixos = !desenhaEixos;
        break;
        case 'c':
            campoVisaoPrint();
            break;
        case 'm':
            minMaxPrint();
            break;
        case 'a':
            campoDeVisaoConstante += 0.01;
            break;
        case 'd':
            campoDeVisaoConstante -= 0.01;
            break;
        case 'e':
            desenhaEnvelopeOnOff();
            if(desenhaEnvelope == true || desenhaForcaBruta == true)
                cout<<"Modo: Envelope - Nro de Testes ForcaBruta: " << countForcaBruta << " - Testes Envelope X Ponto: " << countEnvelope <<endl;
                countForcaBruta = 0;
            break;
        case 'b':
            desenhaForcaBrutaOnOff();
            if(desenhaForcaBruta)
                cout<<"Modo: Forca Bruta - Nro de Testes ForcaBruta: " << countForcaBruta <<endl;
            break;
        case 'q':
            quadTreeOnOff();
            if(desenhaForcaBruta == true || quadTreeOn ==true)
                cout<<"Modo: QuadTree - Nro de Testes ForcaBruta: " << countForcaBruta << " - Testes Envelope X envelope: " << countQuadTree <<endl;
                countForcaBruta = 0;
            break;

		default:
			break;
	}
    PosicionaTrianguloDoCampoDeVisao();
    glutPostRedisplay();
}
// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
        case GLUT_KEY_LEFT:       // Se pressionar LEFT
            AnguloDoCampoDeVisao+=2;
            break;
        case GLUT_KEY_RIGHT:       // Se pressionar RIGHT
            AnguloDoCampoDeVisao-=2;
            break;
		case GLUT_KEY_UP:
            AvancaCampoDeVisao(2);
            break;
	    case GLUT_KEY_DOWN:
            AvancaCampoDeVisao(-2);
			break;
		default:
			break;
	}
    PosicionaTrianguloDoCampoDeVisao();
    //cout << "Triangulo Base: " << endl;
    //TrianguloBase.imprimeVertices();
    glutPostRedisplay();
}
// **********************************************************************
// Esta função captura o clique do botao direito do mouse sobre a área de
// desenho e converte a coordenada para o sistema de referência definido
// na glOrtho (ver função reshape)
// Este código é baseado em http://hamala.se/forums/viewtopic.php?t=20
// **********************************************************************
void Mouse(int button,int state,int x,int y)
{
    GLint viewport[4];
    GLdouble modelview[16],projection[16];
    GLfloat wx=x,wy,wz;
    GLdouble ox=0.0,oy=0.0,oz=0.0;

    if(state!=GLUT_DOWN)
      return;
    if(button!=GLUT_RIGHT_BUTTON)
     return;
    cout << "Botao da direita! ";

    glGetIntegerv(GL_VIEWPORT,viewport);
    y=viewport[3]-y;
    wy=y;
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    glReadPixels(x,y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&wz);
    gluUnProject(wx,wy,wz,modelview,projection,viewport,&ox,&oy,&oz);
    PontoClicado = Ponto(ox,oy,oz);
    FoiClicado = true;
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
// **********************************************************************
int  main ( int argc, char** argv )
{
    cout << "Programa OpenGL" << endl;

    glutInit            ( &argc, argv );
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0,0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize  ( 500, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de titulo da janela.
    glutCreateWindow    ( "Poligonos em OpenGL" );

    // executa algumas inicializaÁıes
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // ser· chamada automaticamente quando
    // for necess·rio redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalidação da tela. A funcao "display"
    // ser· chamada automaticamente sempre que a
    // máquina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // ser· chamada automaticamente quando
    // o usu·rio alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // ser· chamada automaticamente sempre
    // o usu·rio pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" ser· chamada
    // automaticamente sempre o usu·rio
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    glutMouseFunc(Mouse);

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
