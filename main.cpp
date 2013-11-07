#include <cmath>
#include <cstdlib>
#include <iostream>

// �����̌v���� glfwGetTime() ���g���Ȃ� 1
#define USE_GLFWGETTIME 0

// �⏕�v���O����
#include "gg.h"
using namespace gg;

// Alias OBJ �`���̌`��f�[�^
#include "GgObj.h"

// Alias OBJ �`���̌`��f�[�^�p�V�F�[�_
#include "GgObjShader.h"

// �`��f�[�^
//static const char objfile[] = "box.obj";
//static const char objfile[] = "ball.obj";
static const char objfile[] = "bunny.obj";
//static const char objfile[] = "dragon2.obj";
//static const char objfile[] = "budda.obj";
//static const char objfile[] = "HondaS2000.obj";
//static const char objfile[] = "AC 1038.obj";

// �e�N�X�`��
static const char *texfile[] =
{
  "reflection0.tga",
  "reflection1.tga",
  "reflection2.tga",
  "reflection3.tga",
  "reflection4.tga",
  "reflection5.tga",
  "reflection6.tga",
  "reflection7.tga",
  "reflection8.tga",
  "reflection9.tga"
};
static const int texfiles = sizeof texfile / sizeof texfile[0];
static int texselect = 1;
static GLuint texname[texfiles];

// �t���[���o�b�t�@�I�u�W�F�N�g�̃T�C�Y
#define FBOWIDTH 1024
#define FBOHEIGHT 1024

// �[�x�̃T���v�����O�Ɏg���_�Q��
#define MAXSAMPLES 256

// �T���v���_�̐�
static GLint samples = 64;

// �T���v���_�̎U�z���a
static GLint radius = 20;

// �������x
static GLint translucency = 50;

// �������x
static GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };

// �E�B���h�E�T�C�Y
static int width = 800, height = 800;

// �������e�ϊ��s��
static GgMatrix mp;

// �g���b�N�{�[��
static GgTrackball tbl, tbr;

// �x���`�}�[�N
static bool benchmark = false;

//
// �E�B���h�E�̃T�C�Y�ύX���̏���
//
static void GLFWCALL resize(int w, int h)
{
  // �E�B���h�E�T�C�Y��ۑ�����
  width = w;
  height = h;

  // �E�B���h�E�S�̂��r���[�|�[�g�ɂ���
  glViewport(0, 0, w, h);

  // �������e�ϊ��s������߂�i�A�X�y�N�g�� w / h�j
  mp.loadPerspective(0.6f, (GLfloat)w / (GLfloat)h, 2.5f, 5.5f);
  //mp.loadPerspective(0.6f, (GLfloat)w / (GLfloat)h, 2.5f, 5.0f);

  // �g���b�N�{�[�������͈̔͂�ݒ肷��
  tbl.region(w, h);
  tbr.region(w, h);
}

//
// �}�E�X�{�^�����쎞�̏���
//
static void GLFWCALL mouse(int button, int action)
{
  // �}�E�X�̌��݈ʒu���擾����
  int x, y;
  glfwGetMousePos(&x, &y);

  switch (button)
  {
  case GLFW_MOUSE_BUTTON_LEFT:
    if (action != GLFW_RELEASE)
    {
      // ���{�^������
      tbl.start(x, y);
    }
    else
    {
      // ���{�^���J��
      tbl.stop(x, y);
    }
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    if (action != GLFW_RELEASE)
    {
      // �E�{�^������
      tbr.start(x, y);
    }
    else
    {
      // �E�{�^���J��
      tbr.stop(x, y);
    }
    break;
  default:
    break;
  }
}

//
// �L�[�{�[�h
//
static void GLFWCALL keyboard(int key, int action)
{
  if (action == GLFW_PRESS)
  {
    if (key >= '0' && key <= '9')
    {
      texselect = key - '0';
    }
    else
    {
      switch (key)
      {
      case GLFW_KEY_SPACE:
        if (samples < MAXSAMPLES) ++samples;
        std::cout << "SAMPLES= " << samples << std::endl;
        break;
      case GLFW_KEY_BACKSPACE:
      case GLFW_KEY_DEL:
        if (samples > 0) --samples;
        std::cout << "SAMPLES= " << samples << std::endl;
        break;
      case GLFW_KEY_UP:
        std::cout << "TRANSLUCENCY= " << ++translucency * 0.01f << std::endl;
        break;
      case GLFW_KEY_DOWN:
        std::cout << "TRANSLUCENCY= " << --translucency * 0.01f << std::endl;
        break;
      case GLFW_KEY_RIGHT:
        std::cout << "RADIUS= " << ++radius * 0.01f << std::endl;
        break;
      case GLFW_KEY_LEFT:
        std::cout << "RADIUS= " << --radius * 0.01f << std::endl;
        break;
      case 'b':
      case 'B':
        benchmark = true;
        break;
      case GLFW_KEY_ESC:
      case 'Q':
      case 'q':
        exit(0);
      default:
        break;
      }
    }
  }
}

//
// �����ݒ�
//
static int init(const char *title)
{
  // GLFW ������������
  if (glfwInit() == GL_FALSE)
  {
    // �������Ɏ��s����
    std::cerr << "Error: Failed to initialize GLFW." << std::endl;
    return false;
  }

  // OpenGL Version 3.2 Core Profile ��I������
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

  // �E�B���h�E���J��
  if (glfwOpenWindow(width, height, 8, 8, 8, 8, 24, 8, GLFW_WINDOW) == GL_FALSE)
  {
    // �E�B���h�E���J���Ȃ�����
    std::cerr << "Error: Failed to open GLFW window." << std::endl;
    return false;
  }

  // �J�����E�B���h�E�ɑ΂���ݒ�
  glfwEnable(GLFW_KEY_REPEAT);
  glfwSwapInterval(1);
  glfwSetWindowTitle(title);

  // �E�B���h�E�̃T�C�Y�ύX���ɌĂяo�������̐ݒ�
  glfwSetWindowSizeCallback(resize);

  // �}�E�X�̃{�^�����쎞�ɌĂяo�������̐ݒ�
  glfwSetMouseButtonCallback(mouse);

  // �L�[�{�[�h���쎞�ɌĂяo�������̐ݒ�
  glfwSetKeyCallback(keyboard);

  // �⏕�v���O�����ɂ�鏉����
  ggInit();

  // �����ݒ�
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glEnable(GL_BLEND);
  //glEnable(GL_MULTISAMPLE);
  //glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
  //glEnable(GL_CULL_FACE);

  // �t���[���o�b�t�@�I�u�W�F�N�g�̏����l
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // �e�N�X�`���t�@�C���̓ǂݍ���
  glGenTextures(texfiles, texname);
  for (int i = 0; i < texfiles; ++i)
  {
    // �e�N�X�`���̌���
    glBindTexture(GL_TEXTURE_2D, texname[i]);

    // �摜�t�@�C���̓ǂݍ���
    ggLoadImage(texfile[i]);
  }

  return true;
}

//
// �t���[���o�b�t�@�I�u�W�F�N�g�̃A�^�b�`�����g�̍쐬
//
static GLuint attachTexture(GLint internal, GLint format, GLsizei width, GLsizei height, GLenum attachment)
{
  // �e�N�X�`���I�u�W�F�N�g�̍쐬
  GLuint texture;
  glGenTextures(1, &texture);

  // �e�N�X�`���̌���
  glBindTexture(GL_TEXTURE_2D, texture);

  // �e�N�X�`���������̊m��
  ggLoadTexture(width, height, internal, format);

  // �t���[���o�b�t�@�I�u�W�F�N�g�Ɍ���
  glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);

  return texture;
}

//
// �t���[���o�b�t�@�I�u�W�F�N�g�̏���
//
static GLuint prepareFBO(const GLenum *buf, GLuint bufnum, GLuint *tex)
{
  // �t���[���o�b�t�@�I�u�W�F�N�g�̍쐬
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // �g�U���ˌW�� D
  tex[0] = attachTexture(GL_RGBA, GL_RGBA, FBOWIDTH, FBOHEIGHT, buf[0]);

  // ���ʔ��ˌW�� S
  tex[1] = attachTexture(GL_RGBA, GL_RGBA, FBOWIDTH, FBOHEIGHT, buf[1]);

  // �ʒu P
  tex[2] = attachTexture(GL_RGB16F, GL_RGB, FBOWIDTH, FBOHEIGHT, buf[2]);

  // �@�� N
  tex[3] = attachTexture(GL_RGB16F, GL_RGB, FBOWIDTH, FBOHEIGHT, buf[3]);

  // �[�x
  tex[4] = attachTexture(GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, FBOWIDTH, FBOHEIGHT, GL_DEPTH_ATTACHMENT);

  return fbo;
}

//
// ���C���v���O����
//
int main(int argc, const char * argv[])
{
  // �����ݒ�
  if (!init("Irradiance Map Screen Space Sub-Surface Scattaring")) return 1;

  // ��1�p�X�̃V�F�[�_
  GgObjShader pass1("pass1.vert", "pass1.frag");

  // ��2�p�X�̃V�F�[�_
  GLuint program = ggLoadShader("pass2.vert", "pass2.frag");
  GLint samplesLoc = glGetUniformLocation(program, "samples");
  GLint radiusLoc = glGetUniformLocation(program, "radius");
  GLint translucencyLoc = glGetUniformLocation(program, "translucency");
  GLint ambientLoc = glGetUniformLocation(program, "ambient");
  GLint unitLoc = glGetUniformLocation(program, "unit");
  GLint pointLoc = glGetUniformLocation(program, "point");
  GLint mpLoc = glGetUniformLocation(program, "mp");
  GLint mrLoc = glGetUniformLocation(program, "mr");
  GLint mtLoc = glGetUniformLocation(program, "mt");

  // �e�N�X�`�����j�b�g
  GLint unit[7];
  for (GLint i = 0; i < 7; ++i) unit[i] = i;

  // �T���v���_
  GLfloat point[MAXSAMPLES][4];
  for (int i = 0; i < MAXSAMPLES; ++i)
  {
    float r = pow((float)rand() / (float)RAND_MAX, 1.0f / 3.0f);
    //float r = (float)rand() / (float)RAND_MAX;
    float t = 6.2831853f * (float)rand() / ((float)RAND_MAX + 1.0f);
    float cp = 2.0f * (float)rand() / (float)RAND_MAX - 1.0f;
    float sp = sqrt(1.0f - cp * cp);
    float ct = cos(t), st = sin(t);

    point[i][0] = sp * ct;
    point[i][1] = sp * st;
    point[i][2] = cp;
    point[i][3] = r;
  }

  // �����ʃ}�b�s���O�p�̃e�N�X�`���ϊ��s��
  static const GLfloat mr[] =
  {
    -1.0f,  0.0f,  0.0f,  0.0f,
    1.0f,  1.0f,  0.0f,  2.0f,
    0.0f, -1.0f,  0.0f,  0.0f,
    1.0f,  1.0f,  0.0f,  2.0f,
  };

  // OBJ �t�@�C��
  GgObj obj(objfile, true);
  obj.attachShader(pass1);

  // �\���̈�𕢂���`
  static const GLfloat position[4][3] =
  {
    { -1.0f, -1.0f, 0.0f },
    {  1.0f, -1.0f, 0.0f },
    { -1.0f,  1.0f, 0.0f },
    {  1.0f,  1.0f, 0.0f }
  };
  GgPoints rect(4, position, GL_TRIANGLE_STRIP);

  // �����_�[�^�[�Q�b�g
  static const GLenum buf[] =
  {
    GL_COLOR_ATTACHMENT0, // �g�U���ˌ�
    GL_COLOR_ATTACHMENT1, // ����
    GL_COLOR_ATTACHMENT2, // �ʒu
    GL_COLOR_ATTACHMENT3, // �@��
  };

  // �����_�[�^�[�Q�b�g�̃A�^�b�`�����g�̐�
  static const GLsizei bufnum = sizeof buf / sizeof buf[0];

  // �K�v�ɂȂ�e�N�X�`���̐��i�����_�[�^�[�Q�b�g�{�[�x�{���j
  static const GLsizei texnum = bufnum + 2;

  // �e�N�X�`��
  GLuint tex[texnum];

  // FBO �̏���
  GLuint fbo = prepareFBO(buf, bufnum, tex);

  // �r���[�ϊ��s��� mv �ɋ��߂�
  GgMatrix mv = ggTranslate(0.0f, 0.0f, -4.0f);
  //GgMatrix mv = ggLookat(2.0f, 1.0f, 2.5f, 0.0f, -0.1f, 0.0f, 0.0f, 1.0f, 0.0f);
  //GgMatrix mv = ggLookat(-3.0f, 1.0f, 2.5f, 0.0f, -0.1f, 0.0f, 0.0f, 1.0f, 0.0f);

  // ���Ԍv���p�� Query Object
  GLuint query;
  glGenQueries(1, &query);

  // �o�ߎ��Ԃ̍��v
  double tstep1 = 0.0, tstep2 = 0.0;
#if USE_GLFWGETTIME
  double sstep1 = 0.0, sstep2 = 0.0;
#endif

  // �t���[����
  int frames = 0;

  // �E�B���h�E���J���Ă���Ԃ���Ԃ��`�悷��
  while (glfwGetWindowParam(GLFW_OPENED))
  {
#if USE_GLFWGETTIME
    // ���ݎ���
    double s0, s1, s2;
#endif

    // ���Ԃ̌v��
    if (benchmark)
    {
#if USE_GLFWGETTIME
      glFinish();
      s0 = glfwGetTime();
#endif

      glBeginQuery(GL_TIME_ELAPSED, query);
    }

    // �t���[���o�b�t�@�I�u�W�F�N�g�ɕ`��
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // �����_�[�^�[�Q�b�g�̎w��
    glDrawBuffers(bufnum, buf);

    // �r���[�|�[�g�� FBO �̃T�C�Y�ɍ��킹��
    glViewport(0, 0, FBOWIDTH, FBOHEIGHT);

    // �B�ʏ�����L���ɂ���
    glEnable(GL_DEPTH_TEST);

    // ��ʏ���
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �V�[���̕`��
    pass1.loadMatrix(mp, mv * tbl.get());
    obj.draw();

    // ���Ԃ̌v��
    if (benchmark)
    {
#if USE_GLFWGETTIME
      glFinish();
      s1 = glfwGetTime();
#endif
      glEndQuery(GL_TIME_ELAPSED);

      GLint done;
      do { glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done); } while (!done);

      GLuint64 elapsed_time;
      glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
      tstep1 += static_cast<double>(elapsed_time) * 0.000001;

      glBeginQuery(GL_TIME_ELAPSED, query);
    }

    // �ʏ�̃t���[���o�b�t�@�ɕ`��
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // �ʏ�̃t���[���o�b�t�@�i�o�b�N�o�b�t�@�j���w�肷��
    glDrawBuffer(GL_BACK);

    // �r���[�|�[�g�� FBO �̃T�C�Y�ɍ��킹��
    glViewport(0, 0, width, height);

    // �B�ʏ����𖳌��ɂ���
    glDisable(GL_DEPTH_TEST);

    // ���̃e�N�X�`��
    tex[5] = texname[texselect];

    // �x�������_�����O
    glUseProgram(program);
    glUniform1i(samplesLoc, samples);
    glUniform1f(radiusLoc, radius * 0.01f);
    glUniform1f(translucencyLoc, translucency * 0.01f);
    glUniform4fv(ambientLoc, 1, ambient);
    glUniform1iv(unitLoc, texnum, unit);
    glUniform4fv(pointLoc, MAXSAMPLES, point[0]);
    glUniformMatrix4fv(mpLoc, 1, GL_FALSE, mp.get());
    glUniformMatrix4fv(mrLoc, 1, GL_FALSE, mr);
    glUniformMatrix4fv(mtLoc, 1, GL_FALSE, tbr.get());
    for (GLsizei i = 0; i < texnum; ++i)
    {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, tex[i]);
    }
    rect.draw();

    // ���Ԃ̌v��
    if (benchmark)
    {
#if USE_GLFWGETTIME
      glFinish();
      s2 = glfwGetTime();
#endif
      glEndQuery(GL_TIME_ELAPSED);

      GLint done;
      do { glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done); } while (!done);

      GLuint64 elapsed_time;
      glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
      tstep2 += static_cast<double>(elapsed_time) * 0.000001;
    }

    // ���s���Ԃ̏o��
    if (benchmark)
    {
      ++frames;

      std::cout << frames << ": "
        << tstep1 / frames
#if USE_GLFWGETTIME
        << " (" << (sstep1 += static_cast<double>(s1 - s0)) * 1000.0 / static_cast<double>(frames) << ")"
#endif
        << ", "
        << tstep2 / frames
#if USE_GLFWGETTIME
        << " (" << (sstep2 += static_cast<double>(s2 - s1)) * 1000.0 / static_cast<double>(frames) << ")"
#endif
        << std::endl;
    }

    // �o�b�t�@�����ւ���
    glfwSwapBuffers();

    // �}�E�X����Ȃǂ̃C�x���g�ҋ@
    glfwWaitEvents();

    // �}�E�X�̌��݈ʒu���擾����
    int mx, my;
    glfwGetMousePos(&mx, &my);

    // ���}�E�X�{�^���h���b�O
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE) tbl.motion(mx, my);

    // �E�}�E�X�{�^���h���b�O
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) != GLFW_RELEASE) tbr.motion(mx, my);
  }

  return 0;
}