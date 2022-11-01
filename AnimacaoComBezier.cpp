// **********************************************************************
// PUCRS/Escola PolitŽcnica
// COMPUTA‚ÌO GRçFICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
//

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
#include "InstanciaBZ.h"

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"

Temporizador T;
double AccumDeltaT=0;
Temporizador T2;
ifstream arquivo;

InstanciaBZ Personagens[10];

Ponto CurvasPersonagem[10][3];

Bezier Curvas[20];
unsigned int nCurvas;

// Limites l—gicos da ‡rea de desenho
Ponto Min, Max;

bool desenha = false;

Poligono Mapa, MeiaSeta, Mastro, Pontos;
vector<vector<int> > curvasBezier;
int nInstancias=0;

Ponto movimento(0,1,0);
int xr,yr,auxr;
int caso = 1;
int ant;

int nInimigos=10;
//int personagemPrincipal[5][5];

int curvAtual = 0;

float angulo=0.0;


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

    if (AccumDeltaT > 1.0/30) // fixa a atualiza‹o da tela em 30
    {
        AccumDeltaT = 0;
        angulo+=2;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        cout << "Tempo Acumulado: "  << TempoTotal << " segundos. " ;
        cout << "Nros de Frames sem desenho: " << nFrames << endl;
        cout << "FPS(sem desenho): " << nFrames/TempoTotal << endl;
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
    glOrtho(Min.x,Max.x, Min.y,Max.y, -10,+10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
// **********************************************************************
// **********************************************************************
void DesenhaEixos()
{
    Ponto Meio;
    Meio.x = (Max.x+Min.x)/2;
    Meio.y = (Max.y+Min.y)/2;
    Meio.z = (Max.z+Min.z)/2;

    glBegin(GL_LINES);
    //  eixo horizontal
        glVertex2f(Min.x,Meio.y);
        glVertex2f(Max.x,Meio.y);
    //  eixo vertical
        glVertex2f(Meio.x,Min.y);
        glVertex2f(Meio.x,Max.y);
    glEnd();
}

// **********************************************************************
void DesenhaSeta()
{
    glPushMatrix();
        Mastro.desenhaPoligono();
    glPopMatrix();
}
// **********************************************************************
void DesenhaApontador()
{
    glPushMatrix();
        glTranslated(0, 0, 0);
        DesenhaSeta();
    glPopMatrix();
}
// **********************************************************************
//void DesenhaHelice()
//{
//    glPushMatrix();
//    for(int i=0;i < 4; i++)
//    {
//        glRotatef(90, 0, 0, 1);
//        DesenhaApontador();
//    }
//    glPopMatrix();
//}
// **********************************************************************
//void DesenhaHelicesGirando()
//{
//    glPushMatrix();
//        glRotatef(angulo, 0, 0, 1);
//        DesenhaHelice();
//   glPopMatrix();
//}
// **********************************************************************
//void DesenhaMastro()
//{
//    Mastro.desenhaPoligono();
//}
// **********************************************************************
void DesenhaCatavento()
{
    glLineWidth(3);
    //glPushMatrix();
        //defineCor(BrightGold);
        //DesenhaMastro();
    glPushMatrix();
        glColor3f(1,0,0); // R, G, B  [0..1]
        glTranslated(0,0,0);
        glScaled(0.2, 0.2, 1);
        defineCor(YellowGreen);
        DesenhaApontador();

    glPopMatrix();
}
// **********************************************************************
// Esta fun‹o deve instanciar todos os personagens do cen‡rio
// **********************************************************************
void CriaInstancias(int i)
{
    Personagens[i].Posicao = Pontos.getVertice(i);
    Personagens[i].Rotacao = -90;
    Personagens[i].modelo = DesenhaCatavento;
    Personagens[i].Escala = Ponto (1,1,1);
    Personagens[i].Curva = Bezier(Curvas[0].getPC(0), Curvas[0].getPC(1), Curvas[0].getPC(2));


    //Personagens[1].Posicao = Ponto (3,0);
    //Personagens[1].Rotacao = -90;
    //Personagens[1].modelo = DesenhaCatavento;

    //Personagens[2].Posicao = Ponto (0,-5);
    //Personagens[2].Rotacao = 0;
    //Personagens[2].modelo = DesenhaCatavento;

    nInstancias = 5;

}
// **********************************************************************
//
// **********************************************************************
void CarregaModelos()
{
    //Mapa.LePoligono("EstadoRS.txt");
    //MeiaSeta.LePoligono("MeiaSeta.txt");
    Mastro.LePoligono("Mastro.txt");
    Pontos.LePoligono("PontosDeControle.txt");


}



void CriaCurvas()
{
    arquivo.open("curvas.txt");
    int pos;
    vector<int> aux;
    arquivo >> nCurvas;
    for (int i = 0; i < nCurvas; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            arquivo >> pos;
            aux.push_back(pos);
            if(aux.size() == 3)
            {
                curvasBezier.push_back(aux);
                aux.clear();
            }
        }
        Curvas[i] = Bezier(Pontos.getVertice(curvasBezier[i][0]), Pontos.getVertice(curvasBezier[i][1]), Pontos.getVertice(curvasBezier[i][2]));
    }

}
// **********************************************************************
//
// **********************************************************************
void init()
{
    // Define a cor do fundo da tela (AZUL)
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    CarregaModelos();
    CriaCurvas();

    for (int i =0; i < 5; i++)
    {
       CriaInstancias(i);
    }


    float d = 5;
    Min = Ponto(-d,-d);
    Max = Ponto(d,d);
}

// **********************************************************************

void DesenhaRetangulo()
{
    glBegin(GL_QUADS);
        glVertex2d(-0.5, -0.5);
        glVertex2d(-0.5, 0.5);
        glVertex2d(0.5, 0.5);
        glVertex2d(0.5, -0.5);
    glEnd();
}

void DesenhaPersonagens(float tempoDecorrido)
{
    //cout << "nInstancias: " << nInstancias << endl;
    for (int i = 0; i < nInstancias; i++)
    {
        Personagens[i].AtualizaPosicao(tempoDecorrido);
        Personagens[i].desenha();

    }
}
// **********************************************************************
//
// **********************************************************************
void DesenhaCurvas()
{
    for(int i=0; i<nCurvas;i++)
    {
        ListaDeCoresRGB::defineCor(Curvas[i].cor);
        Curvas[i].Traca();
    }
}

void movePersonagens(int personagem)
{


    double deltaT = 1.0/50;
    double T = deltaT;
    Bezier aux;


    Personagens[personagem].Velocidade = 1;

    if (Personagens[personagem].tAtual <= 1.0 && Personagens[personagem].tAtual >= 0.0)
    {
        float dt = Personagens[personagem].Velocidade/Curvas[curvAtual].ComprimentoTotalDaCurva;
        double deslocamento = Personagens[personagem].Velocidade*dt;

        T = Curvas[curvAtual].CalculaT(deslocamento);

        Curvas[curvAtual].Calcula(T);

        //cout << "Deslocamento: " << deslocamento << endl;

        cout << "Caso: " << caso << endl;



        Personagens[personagem].tAtual += T;




        cout << Personagens[personagem].tAtual << endl;


        Ponto P = Curvas[curvAtual].Calcula(Personagens[personagem].tAtual);
        Personagens[personagem].Posicao = P;
        P.imprime();
        cout << "Curva Atual: " << curvAtual << endl;


    }

    if (Personagens[personagem].tAtual > 1.0)
    {
        cout << "entrei" << endl;
        for(int i = 0; i < nCurvas; i++)
        {
            if (Curvas[curvAtual].getPC(2) == Curvas[i].getPC(0) && i != curvAtual)
            {
                curvAtual = i;
                Personagens[personagem].tAtual = 0;
                i = nCurvas;
            }
        }

    }

    if (Personagens[personagem].tAtual < 0.0)
    {
        for(int j = 0; j < nCurvas; j++)
        {
            if (Curvas[curvAtual].getPC(0) == Curvas[j].getPC(2) && j != curvAtual)
            {
                curvAtual = j;
                Personagens[personagem].tAtual = 1;
                j = nCurvas;
            }
        }

    }





    /*
    else if (Personagens[personagem].tAtual > 0 && (caso == 0 || caso == 2))
    {
        float dt = Personagens[personagem].Velocidade/Curvas[curvAtual].ComprimentoTotalDaCurva;
        double deslocamento = Personagens[personagem].Velocidade*dt;

        T = Curvas[curvAtual].CalculaT(deslocamento);

        Curvas[curvAtual].Calcula(T);

        Personagens[personagem].tAtual -= T;

        cout << Personagens[personagem].tAtual << endl;


        Ponto P = Curvas[curvAtual].Calcula(Personagens[personagem].tAtual);
        Personagens[personagem].Posicao = P;
        P.imprime();
        cout << "Curva Atual: " << curvAtual << endl;


    }

    if (Personagens[personagem].tAtual > 1.0 && caso == 1)
    {
        for(int i = 0; i < nCurvas; i++)
        {
            if (Curvas[curvAtual].getPC(1) == Curvas[i].getPC(0) && i != curvAtual)
            {
                curvAtual = i;
                Personagens[personagem].tAtual = 1;
                caso = 0;
                i = nCurvas;
            }
        }

    }

    if (Personagens[personagem].tAtual < 0 && caso == 0)
    {
       for(int j = 0; j < nCurvas; j++)
        {
            if (Curvas[curvAtual].getPC(0) == Curvas[j].getPC(1) && j != curvAtual)
            {
                curvAtual = j;
                Personagens[personagem].tAtual = 0;
                caso = 2;
                j = nCurvas;
            }
        }
    }

    if (Personagens[personagem].tAtual < 1.0 && caso == 2)
    {
        for(int i = 0; i < nCurvas; i++)
        {
            if (Curvas[curvAtual].getPC(1) == Curvas[i].getPC(0) && i != curvAtual)
            {
                curvAtual = i;
                Personagens[personagem].tAtual = 1;
                caso = 1;
                i = nCurvas;
            }
        }

    }
    */

}


// **********************************************************************
//  void display( void )
// **********************************************************************
void display( void )
{

	// Limpa a tela coma cor de fundo
	glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites lógicos da área OpenGL dentro da Janela
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// Coloque aqui as chamadas das rotinas que desenham os objetos
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	glLineWidth(1);
	glColor3f(1,1,1); // R, G, B  [0..1]


    DesenhaEixos();

    DesenhaPersonagens(T2.getDeltaT());
    DesenhaCurvas();

    movePersonagens(0);



	glutSwapBuffers();
}
// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo nœmero de segundos e informa quanto frames
// se passaram neste per’odo.
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
// **********************************************************************
void keyboard ( unsigned char key, int x, int y )
{

	switch ( key )
	{
		case 27:        // Termina o programa qdo
			exit ( 0 );   // a tecla ESC for pressionada
			break;
        case 't':
            ContaTempo(3);
            break;
        case ' ':
            desenha = !desenha;
        break;
		default:
			break;
	}
}
// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
        case GLUT_KEY_LEFT:
            Personagens[1].Posicao.x -= 0.5;
            break;
        case GLUT_KEY_RIGHT:
            Personagens[1].Rotacao++;
            break;
		case GLUT_KEY_UP:       // Se pressionar UP
			glutFullScreen ( ); // Vai para Full Screen
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar UP
								// Reposiciona a janela
            glutPositionWindow (50,50);
			glutReshapeWindow ( 700, 500 );
			break;
		default:
			break;
	}
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
    glutInitWindowSize  ( 650, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de título da janela.
    glutCreateWindow    ( "Animacao com Bezier" );

    // executa algumas inicializações
    init ();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // será chamada automaticamente quando
    // for necessário redesenhar a janela
    glutDisplayFunc ( display );

    // Define que o tratador de evento para
    // o invalida‹o da tela. A funcao "display"
    // será chamada automaticamente sempre que a
    // m‡quina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // será chamada automaticamente quando
    // o usuário alterar o tamanho da janela
    glutReshapeFunc ( reshape );

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // será chamada automaticamente sempre
    // o usuário pressionar uma tecla comum
    glutKeyboardFunc ( keyboard );

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" será chamada
    // automaticamente sempre o usuário
    // pressionar uma tecla especial
    glutSpecialFunc ( arrow_keys );

    // inicia o tratamento dos eventos
    glutMainLoop ( );

    return 0;
}
