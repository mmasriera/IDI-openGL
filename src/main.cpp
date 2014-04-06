
#if defined(__APPLE__)
        #include <OpenGL/OpenGL.h>
        #include <GLUT/GLUT.h>
#else
        #include <GL/gl.h>
        #include <GL/glu.h>
        #include <GL/freeglut.h>
#endif
#include "iostream"
#include "model.h"
#include"math.h"
#define _USE_MATH_DEFINES
using namespace std;


Model m;
int alt, amp; // de la finestra
int grausX, grausY;
char estat;
bool paretsVisibles;
double relacioAspecte;
double radiEsferaMinima;
double dist;
bool ortogonal;
bool modeEuler;
double ox, oy, oz, vx, vy, vz, upx, upy, upz;
double vecX, vecY, vecZ;
double Znear, Zfar;
double zoom;
double porscheX, porscheZ, porscheAngle;


bool modeWalk=false;

double walkOx = 0.0;
double walkOz = 0.0;

double walkVx = 0.0;
double walkVz = 1.0;

double velocitat = 0.2;

double ultimaY; // per el zoom

bool llumActiva;

bool perVertex; // per indicar si pintem per vertex o no

bool lght0, lght1, lght2;

double posicioCantonada[] = {6.0, 6.0}; // el 0 es la X i el 1 la Z (altura sempre sera 1)

GLfloat ambient0[] = { 0.1, 0.1, 0.1, 1.0 };
GLfloat difussa0[] ={ 1.0, 1.0, 0.0, 1.0};

GLfloat especular012[] ={ 0.9, 0.7, 0.7, 1.0};

GLfloat ambient12[] = { 0.05, 0.05, 0.05, 1.0};
GLfloat difusa12[] ={ 1.0, 1.0, 1.0, 1.0};



struct caixaMinima{
    double minX, minY, minZ, maxX, maxY, maxZ;
};

caixaMinima cm;

void inicialitzaCaixaMinima(){

    //EM RECORRO EL VECTOR DE VERTEX A SACO, perque es especific de cada model,
    // el recorres tot sabent que estan les x, y i z en ordre
    cm.maxX = cm.minX = m.vertices()[0];
    cm.maxY = cm.minY = m.vertices()[1];
    cm.maxZ = cm.minZ = m.vertices()[2];

    for( int i = 3; i < m.vertices().size(); i+=3 ){ // vector de faces
            double xProvisional = m.vertices()[ i ]; // x
            double yProvisional = m.vertices()[ i+1]; // y
            double zProvisional = m.vertices()[ i+2]; // z

            if( xProvisional > cm.maxX ) cm.maxX = xProvisional;
            if( xProvisional < cm.minX ) cm.minX = xProvisional;
            if( yProvisional > cm.maxY ) cm.maxY = yProvisional;
            if( yProvisional < cm.minY ) cm.minY = yProvisional;
            if( zProvisional > cm.maxZ ) cm.maxZ = zProvisional;
            if( zProvisional < cm.minZ ) cm.minZ = zProvisional;
    }

    cout << "Caixa " << cm.maxX << " " << cm.minX << " " << cm.maxY << " " << cm.minY << " " << cm.maxZ << " " << cm.minZ << endl;
}

void inicialitzaEsferaMinima(){

    caixaMinima c;
    c.minX = c.minZ = -5.0;
    c.maxX = c.maxZ = 5.0;
    c.minY = 0.0;
    c.maxY = 1.5; //inicialita amb el terra

    double restaqx = pow((c.maxX-c.minX),2.0);
    double restaqy = pow((c.maxY-c.minY),2.0);
    double restaqz = pow((c.maxZ-c.minZ),2.0);

    radiEsferaMinima = ( sqrt(restaqx + restaqy + restaqz) )/2 ;

    cout << " radiesf -> " << radiEsferaMinima << endl;
}

void escalaModel( double escalaX, double escalaY, double escalaZ ){
    // perque estigui proporcionat es devideix 1/mida maxima
    // sino el deforma perque fa que tots els costats vagin de -1 a 1
    double midaX = ( cm.maxX-cm.minX) ;
    double midaY = ( cm.maxY-cm.minY) ;
    double midaZ = ( cm.maxZ-cm.minZ) ;
    double midamax = max( midaX, midaY );
    midamax = max( midamax, midaZ );
    // 1/midamax fa la relacio a 1, i *2 perque vagi de -1 a 1
    // i * escala per fer la escala que volem * 0.5 per compensar el 2
    glScalef( escalaX*(2/midamax) , escalaY*(2/midamax) , escalaZ*(2/midamax) );
}

void desplacarAlCentre(){
    //desplaço en sentit negatiu el centre de la caixa
    double px = (cm.maxX+cm.minX)/(-2.0);
    //double py = (cm.maxY+cm.minY)/(-2.0); //CENTRE DE LES Y
    double py = cm.minY * -1.0; // per agafar el centre la base
    double pz = (cm.maxZ+cm.minZ)/(-2.0);
    glTranslatef( px, py , pz );
}

void pintaModel(){

    glPushMatrix();

    double movX = porscheX;
    double movY = porscheZ;

    glTranslatef( movX, 0.0 , movY ); // 0.25 es la distancia del centre als peus

    escalaModel( 0.5, 0.5, 0.5); // 2*0.25= 0.5 que demana l enunciat

    glRotatef( porscheAngle, 0, 1, 0 );

    desplacarAlCentre();


    for(int i = 0; i < m.faces().size(); i++){

       Material materialAnterior;
       Material material = Materials[ m.faces()[i].mat];

       // inicialitzo el material anterior
       if( i > 0 )
            materialAnterior = Materials[m.faces()[i-1].mat];

       if( (i == 0) or (materialAnterior.name != material.name) ) {
           glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, material.ambient );
           glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, material.diffuse );
           glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, material.specular );
           glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, material.shininess );
       }

       // si no es per vertex o no hi ha normal per vertex
       if( (!perVertex) or (m.normals().size() == 0) ){
           glNormal3dv( m.faces()[i].normalC );
        }


        glBegin(GL_TRIANGLES);
           for( int vtx = 0; vtx < 3; ++vtx ){
               // si volem normal per vertex I EXISTEIX
                if( perVertex and (m.normals().size() > 0)  )
                      glNormal3dv( &m.normals()[m.faces()[i].n[vtx]]) ;

                glVertex3dv( &m.vertices()[m.faces()[i].v[vtx]]) ;
            }

        glEnd();
    }
    glPopMatrix();
}

void pintaNinotNeu( int posX, int posY, int posZ ){

    GLfloat ambient[] = { 0.3, 0.3, 0.3, 1.0 };
    GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1.0 };
    GLfloat specular[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat shininess = 50;

    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, ambient );
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse );
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, specular );
    glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, shininess );

    glPushMatrix(); //cos
        glTranslatef(posX, 0.4, posZ);
        glColor3f(1.0, 1.0, 1.0);
        glutSolidSphere(0.4, 30, 30);
    glPopMatrix();
    glPushMatrix(); // cap
        glTranslatef(posX, 0.9, posZ);
        glutSolidSphere(0.2, 30, 30);
        glPopMatrix();
    glPushMatrix(); // nas
        glTranslatef(posX, 0.9, posZ);
        glColor3f(1.0, 0.4, 0.0);
        glutSolidCone(0.075, 0.2, 20 , 20);
    glPopMatrix();
}

void pintaTerra(){

    GLfloat ambient[] = { 0.0, 0.0, 0.9, 1.0 };
    GLfloat diffuse[] = { 0.0, 0.0, 0.3, 1.0 };
    GLfloat specular[] = { 0.6, 0.6, 0.6, 1.0 };
    GLfloat shininess = 75.0;

    glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
    glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
    glMaterialf( GL_FRONT, GL_SHININESS, shininess );

    glPushMatrix();
    glColor3f( 0.55, 0.24, 0.2);
    glNormal3f( 0.0, 1.0, 0.0);
    glBegin( GL_QUADS );
        glVertex3f( 5.0, 0.0, 5.0 );
        glVertex3f( 5.0, 0.0, -5.0 );
        glVertex3f( -5.0, 0.0, -5.0 );
        glVertex3f( -5.0, 0.0, 5.0 );
    glEnd();
    glPopMatrix();
}

void pintaParets(){

    GLfloat ambient[] = { 0.0, 0.2, 0.0, 0.0 };
    GLfloat diffuse[] = { 0.0, 0.5, 0.0, 0.0 };
    GLfloat specular[] = { 0.8, 0.8, 0.8, 0.0 };
    GLfloat shininess = 50;

    glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
    glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
    glMaterialf( GL_FRONT, GL_SHININESS, shininess );

    glPushMatrix();
        glColor3f(0.0, 0.6, 0.0);
        glTranslatef( -4.9, 0.75, 0 );
        glScalef( 0.2 , 1.45 , 10.0 );
        glutSolidCube( 1.0 );
    glPopMatrix();

    glPushMatrix();
        glColor3f(0.0, 0.6, 0.0);
        glTranslatef( 2.5, 0.75, -1.5 );
        glScalef( 4.0 , 1.45 , -0.2 );
        glutSolidCube( 1.0 );
    glPopMatrix();
}

void pintaEsfera(){
    glPushMatrix();
        glColor3f(0.0, 0.5, 0.5);
        glutWireSphere( radiEsferaMinima , 30, 30);
    glPopMatrix();
}

void refresh(void) {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glDisable(GL_LIGHT0); glDisable(GL_LIGHT1); glDisable(GL_LIGHT2);

        if(llumActiva) {
            if(lght0){
                glEnable(GL_LIGHT0);
                glLightfv (GL_LIGHT0, GL_AMBIENT, ambient0);
                glLightfv (GL_LIGHT0, GL_DIFFUSE, difussa0);
                glLightfv (GL_LIGHT0, GL_SPECULAR, especular012);

                GLfloat posicio0[] ={ posicioCantonada[0], 1.0 , posicioCantonada[1], 0.0};
                glLightfv(GL_LIGHT0,GL_POSITION, posicio0 );
            }
            if(lght1) {
                glEnable(GL_LIGHT1);
                glLightfv (GL_LIGHT1, GL_AMBIENT, ambient12);
                glLightfv (GL_LIGHT1, GL_DIFFUSE, difusa12);
                glLightfv (GL_LIGHT1, GL_SPECULAR, especular012);

                GLfloat posicio1[] ={ 0.0, 6.0, 0.0, 1.0};
                glLightfv(GL_LIGHT1, GL_POSITION, posicio1);
            }
            if(lght2) {
                glEnable(GL_LIGHT2);
                glLightfv (GL_LIGHT2, GL_AMBIENT, ambient12);
                glLightfv (GL_LIGHT2, GL_DIFFUSE, difusa12);
                glLightfv (GL_LIGHT2, GL_SPECULAR, especular012);

                cout << " posicio del cotxe "<< porscheX << " " <<  porscheZ << endl;
                GLfloat posicio2[] ={ porscheX , 0.4, porscheZ, 0.8};
                glLightfv(GL_LIGHT2, GL_POSITION, posicio2);
            }
        }

        //pintaEsfera();

        pintaTerra();

        pintaModel();

        pintaNinotNeu( 2.5, 0, 2.5);
        pintaNinotNeu( -2.5, 0, 2.5);
        pintaNinotNeu( -2.5, 0, -2.5);
        pintaNinotNeu( 2.5, 0, -2.5);

        if( paretsVisibles ) pintaParets();

        glutSwapBuffers();
}

void initOrtogonal(){

    double migAmple = radiEsferaMinima; // del centre al canto
    double migAlt = radiEsferaMinima; // del centre a dalt

    if( relacioAspecte >= 1 ) // es mes ample que alt
        migAmple = migAmple * relacioAspecte;
    else    // es mes alt que ample
        migAlt = migAlt / relacioAspecte;

    migAlt = migAlt / zoom;
    migAmple = migAmple / zoom;

    glOrtho(
        -migAmple, migAmple, // abaix esquerra
        -migAlt, migAlt,     // adalt dreta
        Znear, Zfar
        );
}

void initPerspectiva(){

    double migAmpleCamera = radiEsferaMinima; // del centre al canto
    double migAltCamera = radiEsferaMinima; // de lcentre a dalt

    double angle = asin( radiEsferaMinima / dist ) / zoom;

    if( relacioAspecte < 1.0 )  // es mes alt que ample
        angle = atan( tan(angle) / relacioAspecte  );

    angle = ( angle*180 ) / M_PI; // pas a radiants

    gluPerspective(
          2*angle,
          relacioAspecte,
          Znear, Zfar
        );
}

void posicionaCamera(){ //GlLookAt

    glLoadIdentity();
    if(modeEuler){

        glTranslatef( 0.0, 0.0, -dist );
        glRotated( 0 , 0.0, 0.0, 0.0); // z
        glRotated( -grausY, 1.0, 0.0, 0.0); // x
        glRotated( grausX, 0.0, 1.0, 0.0); // y
        glTranslatef( 0.0, 0.0, 0.0);

    }

    if( ! modeEuler ) {
        gluLookAt( ox, oy, oz,
              vx, vy, vz,
              upx, upy, upz );
   }
}

void initCamera(){

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

        if( ortogonal )
            initOrtogonal();
        else
            initPerspectiva();

    glMatrixMode(  GL_MODELVIEW );

    posicionaCamera();    //glLookAt

    glutPostRedisplay();
}

void cameraPerDefecte(){
    // posa els valors per defecte i inicialitza la camera
    dist = 2 * radiEsferaMinima;
    Znear = dist - radiEsferaMinima;
    Zfar = dist + radiEsferaMinima;
    zoom = 1.0;

    initCamera();
}

void reshape( int ampleW, int altW){

    cout << "new size: x= " << ampleW << ", y= " << altW << endl;

    // el que vull que tingui el viewPort (es pot posar directament la crida a viewport)

    amp = ampleW;
    alt = altW;

    relacioAspecte = (double) ampleW / (double) altW;
    cout << " relacio d' aspecte " << relacioAspecte << endl;

    initCamera();

    glViewport( 0, 0, ampleW, altW );

    glutPostRedisplay();
}

void motionf(int newx, int newy){

    if ( glutGetModifiers() == GLUT_ACTIVE_SHIFT ) {       //zoom
            zoom +=  (double)newy / (double) alt;
            cout << "zoom = " << zoom << endl;
        }

    else if ( glutGetModifiers() == GLUT_ACTIVE_CTRL ) {       //zoom
        zoom -=  (double)newy / (double) alt;
        if(zoom <=  0.0 ) zoom = 0.01;
        cout << "zoom = " << zoom << endl;
    }

    else if( modeEuler and estat=='e'){      //EULER
        grausX = ((double) newx/ (double) amp )*360;
        grausY = ((double) newy/ (double) alt )*(-360);
    } 
    initCamera();
    glutPostRedisplay();
}

double distanciaDosPunts2D( double p1x, double p1z, double p2x, double p2z){
    double restaX = pow((p1x-p2x),2.0);
    double restaZ = pow((p1z-p2z),2.0);
    return sqrt( restaX + restaZ );
}

void mouEspectador( char t ){

    double vectorUnitari[2] = { vx - ox , vz - oz };

    if( t == 'w'){
        vx += velocitat*vectorUnitari[0];
        vz += velocitat*vectorUnitari[1];

        ox += velocitat*vectorUnitari[0];
        oz += velocitat*vectorUnitari[1];
    }

    if( t == 's'){
        vx -= velocitat*vectorUnitari[0];
        vz -= velocitat*vectorUnitari[1];

        ox -= velocitat*vectorUnitari[0];
        oz -= velocitat*vectorUnitari[1];
    }

    if( t == 'g'){

        double a = porscheAngle / 180*M_PI;

        vx = ox + sin( a );
        vz = oz + cos( a );

    }

    porscheX = ox;
    porscheZ = oz;

    initCamera();
}

void kbrd( unsigned char key, int x, int y ){

    estat = 'n';

    if( key == 'h' ){
        cout << endl << "###############################################################" << endl;
        cout << endl <<" *   f          -> omplir cares" << endl;
        cout << endl <<" *   l          -> veure contorn de les cares" << endl;
        cout << endl <<" *   ESC        -> tancar el programa" << endl;
        cout << endl <<" *   SHIFT+drag -> zoom in" << endl;
        cout << endl <<" *   CTRL+drag  -> zoom out" << endl;
        cout << endl <<" *   c          -> mode walk " << endl;
        cout << endl <<" *   e          -> activar o desactivar el mode angles d'euler (mode euler)" << endl;
        cout << endl <<" *   v          -> parets visibles / no visibles" << endl;
        cout << endl <<" *   r          -> fa un reset de la camera" << endl;
        cout << endl <<" *   x          -> camera ortogonal" << endl;
        cout << endl <<" *   p          -> camera perspectiva " << endl;
        cout << endl <<" *   g          -> augmenta velocitat walk " << endl;
        cout << endl <<" *   b          -> disminueix velocitat walk " << endl;
        cout << endl <<" *   i         -> activar / desactivar la il·luminacio"<< endl;
        cout << endl <<" *   0         -> activar / desactivar la llum 0"<< endl;
        cout << endl <<" *   1         -> activar / desactivar la llum 1"<< endl;
        cout << endl <<" *   2         -> activar / desactivar la llum 2"<< endl;
        cout << endl <<" *   m         -> canviar la posicio de la llum 0"<< endl;
        cout << endl <<"###############################################################" << endl << endl;
    }
    else if( key == 'f' ) {
        cout <<  endl << "  CARES" << endl;   estat = 'f';
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glutPostRedisplay();
    }
    else if(key == 'l'){
        cout <<  endl << "  LINIES" << endl;  estat = 'l';
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glutPostRedisplay();
    }
    else if( key == 'v'){
        cout << endl << "   PARETS" << endl <<  endl; estat = 'v';
        paretsVisibles = ! paretsVisibles;
        glutPostRedisplay();
    }
    else if( key == 'r'){
        cout << endl << "   RESET" << endl << endl; estat = 'r';
        grausX = grausY = 0;
        modeEuler = true;
        ortogonal = false;
        llumActiva=true;
        lght0=false;
        lght1=true;
        lght2=false;
        perVertex=true;
        walkOx = walkOz = walkVx = 0.0;
        walkVz = 1.0;
        porscheAngle = 0.0;
        porscheX = porscheZ = 0.0;

        cameraPerDefecte();
    }
    else if( key == 'x'){
        cout <<  endl << "  CAMERA ORTOGONAL (nomes en mode euler) " << endl << endl; estat = 'x';
        ortogonal = true;
        initCamera();
    }
    else if( key == 'p'){
        cout << endl << "   CAMERA PERSPECTIVA" << endl << endl; estat = 'p';
        ortogonal = false;
        initCamera();
    }
    else if( key == 'e' ){

        if( ! modeEuler ) cout << " no es pot girar amb angles d'euler en mode primera persona " << endl;

        else {
            cout << " mode girs Euler " << endl; estat = 'e';
        }
    }
    else if( key == 'c'){
        cout << endl << "   CAMINAR" << endl; estat = 'c';

        if( modeEuler) { //el meu mde euler es el seu primera perosna

            cout << "  mode primera persona" << endl;
            modeEuler = false;

            modeWalk = true;
            vx = walkVx;
            vy = 0.35;
            vz = walkVz;

            ox = walkOx;
            oy = 0.4;
            oz = walkOz;

            if( ortogonal == false) cout << " P E R S P E C T I V A" << endl;

            upx = 0; upy = 1; upz = 0;
            porscheAngle = 0;
            porscheX = porscheZ = 0;
            Znear = 0.1; Zfar = 20.0;

            initCamera();

        }
        else {

            Znear = dist - radiEsferaMinima;
            Zfar = dist + radiEsferaMinima;
            zoom = 1.0;

            cout << "  mode tercera persona" << endl;
            modeEuler = true;

            cout <<  endl << "  MODE EULERs" << endl << endl;

                modeWalk=false;
                walkVx = vx;
                walkVz = vz;
                walkOx = ox;
                walkOz = oz;
                vx=0;
                vz=0;


            initCamera();
        }

    }
    else if( key == 'w' and (not modeEuler) ){

        cout << "cotxe avança" << endl;

        if( modeWalk  and not ortogonal ){

            mouEspectador('w');
        }

    }
    else if( key == 's' and (not modeEuler)){

        cout << "cotxe retrocedeix" << endl;

        if( modeWalk and  not ortogonal ){

            mouEspectador('s');
        }
    }
    else if( key == 'a' and (not modeEuler)){

        cout << "cotxe gira" << endl;

        porscheAngle += 5;

        mouEspectador('g');
    }

    else if( key == 'd' and (not modeEuler)){

        cout << "cotxe gira" << endl;

        porscheAngle -= 5;

        mouEspectador('g');
    }

    else if( key == 'g' ){

        velocitat += 0.2;
        glutPostRedisplay();
    }

    else if( key == 'b' ){
        
        velocitat -= 0.2;
        if( velocitat < 0.2 ) velocitat = 0.1;
        glutPostRedisplay();
    }

    if( key  == 'm' ){

        if(llumActiva and lght0) {

            cout << "canvia posicio cantonada " << endl;

            if( posicioCantonada[0] == 6.0  ) {
                 if( posicioCantonada[1] == 6.0 ) {
                    posicioCantonada[0] = 6.0; posicioCantonada[1] = -6.0;
                 }
                else {
                    posicioCantonada[0] = -6.0; posicioCantonada[1] = -6.0;
                }
            }
            else {
                if(posicioCantonada[1] == -6.0 ){
                     posicioCantonada[0] = -6.0; posicioCantonada[1] = 6.0;
                }
                else{
                    posicioCantonada[0] = 6.0; posicioCantonada[1] = 6.0;
                }
            }
        }
        glutPostRedisplay();
    }

    else if( key == 'i' ){

        cout << "canvi llumActiva" << endl;

        if(llumActiva) llumActiva=false;
        else llumActiva=true;

        glutPostRedisplay();
    }

    else if( key == '0' ){

        cout << "canvi light0" << endl;

        lght1 = lght2 = false;

        lght0 = ! lght0;

        glutPostRedisplay();
    }

    else if( key == '1' ){

        cout << "canvi light1" << endl;

        lght0 = lght2 = false;

        lght1 = ! lght1;

        glutPostRedisplay();
    }

    else if( key == '2' ){

        lght0 = lght1 = false;

        cout << "canvi light2" << endl;

        lght2 = ! lght2;

        glutPostRedisplay();
    }



    else if( key == 'n' ){

        cout << "canvi de normal a per vertex" << endl;

        if(perVertex) perVertex=false;
        else perVertex =  true;

        glutPostRedisplay();
    }

    else if( key == 27) exit(0); // ESC

    cout <<  "estat " << key << endl;
}

int main(int argc, const char * argv[]) {

        glutInit(&argc, (char**)argv );
        glutInitWindowPosition( 0, 0 );
        glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(600, 600);
        glutCreateWindow("IDI:PractiquesOpenGL");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
        glClearColor(0,0,0,0);
        //glEnable(GL_LIGHT0);
        //glEnable(GL_LIGHT1);

        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


        estat = 'f';
        grausX = grausY = porscheX = porscheZ = porscheAngle =  0;
        porscheX = 0.0; porscheZ = 0.0;
        paretsVisibles = modeEuler = true;
        ortogonal = false;
        perVertex = true;
        llumActiva = true;
        lght0=false;
        lght1=true;
        lght2=false;
        zoom = 1.0;

        cout << endl << "------------------- h  ->  per veure el menu --------------------- " << endl << endl;

        m.load(argv[1]); // l objecte que se li passa per parametre l programa

        inicialitzaCaixaMinima(); //del model
        inicialitzaEsferaMinima(); // de l'escena

        cameraPerDefecte();

        glutDisplayFunc(refresh);
        glutReshapeFunc( reshape ); // resize
        glutMotionFunc( motionf ); //  drag
        glutKeyboardFunc( kbrd ); // teclat

        glutMainLoop();
        return 0;
}
