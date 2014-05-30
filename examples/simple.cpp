
#include <BSystem.h>
#include <BCamera.h>


#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <cstdlib>


baumer::BCamera* g_cam = 0;
baumer::BSystem* g_system  = 0;

GLuint tex;
int width = 640;
int height = 480;

bool flipv = false;
bool fliph = false;


void cleanup(){
}

void keyboard(unsigned char key, int x, int y){
  switch(key){
  case 'v':
    flipv = (flipv+1)%2;
    break;
  case 'h':
    fliph = (fliph+1)%2;
    break;
  case 27:
    cleanup();
    exit(0);
    break;
  case 'G':
    g_cam->setGain(1.05 * g_cam->getGain());
    break;
  case 'g':
    g_cam->setGain(0.95 * g_cam->getGain());
    break;
  case ' ':
    g_cam->saveFrame();
    break;
  case 'q':
    cleanup();
    exit(0);
  default:
    break;
  }
  glutPostRedisplay();
}


void reshape(GLsizei w, GLsizei h)
{
  width = w;
  height = h;
  glutPostRedisplay();
}

void idle(){
  glutPostRedisplay();
}


void display(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (float) width, 0.0, (float) height, 0.1, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, tex);
  {
    boost::mutex::scoped_lock l(g_cam->getMutexLock());
    if(g_cam->getNumChannels() == 3)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_cam->getWidth(), g_cam->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, g_cam->capture());
    else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, g_cam->getWidth(), g_cam->getHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, g_cam->capture());
  }


  glBegin(GL_QUADS);
  if(flipv){
    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.0,0.0,-0.5);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(width,0.0,-0.5);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(width,height,-0.5);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.0, height,-0.5);
  }
  else if(fliph){
    glTexCoord2f(1.0, 1.0);
    glVertex3f(0.0,0.0,-0.5);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(width,0.0,-0.5);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(width,height,-0.5);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(0.0, height,-0.5);
  }
  else{
    glTexCoord2f(0.0, 1.0);
    glVertex3f(0.0,0.0,-0.5);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(width,0.0,-0.5);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(width,height,-0.5);

    glTexCoord2f(0.0, 0.0);
    glVertex3f(0.0, height,-0.5);
  }
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
  

  glutSwapBuffers();
}


int main (int argc, char** argv){


  g_system = new baumer::BSystem;
  g_system->init();
  if(g_system->getNumCameras() == 0){
    std::cerr << "no camera found" << std::endl;
    cleanup();
    return 0;
  }

  g_cam = g_system->getCamera(0 /*the first camera*/, false /*use not rgb*/, false /*not fastest mode but full resolution*/);


  width = g_cam->getWidth();
  height = g_cam->getHeight();

  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(width, height);
  glutInitWindowPosition(0,0);
  glutCreateWindow(argv[0]);

/*
  GLenum err = glewInit();
  if(GLEW_OK != err) {
    fprintf(stderr,"Error: %s\n",glewGetErrorString(err));
    exit(1);
  }
*/


  glutIdleFunc(idle);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);

  glClearColor(0.0,0.0,0.0,0.0);
  glDisable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);


  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  if(g_cam->getNumChannels() == 3)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_cam->getWidth(), g_cam->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, g_cam->getWidth(), g_cam->getHeight(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glutMainLoop();
  cleanup();
  return 0;
}
