#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp> 

using namespace glm;

// ���� ����
int Width = 512;
int Height = 512;
std::vector<float> OutputImage;

// ī�޶� ����
const vec3 eye(0.0f, 0.0f, 0.0f);
const vec3 u(1.0f, 0.0f, 0.0f);
const vec3 v(0.0f, 1.0f, 0.0f);
const vec3 w(0.0f, 0.0f, 1.0f);
const float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, d = 0.1f;
const int nx = 512, ny = 512;

// ����ü �� Ŭ���� ����
struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Intersection {
    bool hit;
    float distance;
    vec3 point;
    vec3 normal;
    vec3 color;
};

class Plane {
public:
    vec3 point;
    vec3 normal;
    vec3 color;

    Plane(vec3 p, vec3 n, vec3 col) : point(p), normal(n), color(col) {}

    Intersection intersect(const Ray& ray) const {
        Intersection result = { false, 0, vec3(0), vec3(0), vec3(0) };
        float denom = dot(normal, ray.direction);
        if (abs(denom) > 1e-6) {
            float t = dot(point - ray.origin, normal) / denom;
            if (t > 0) {
                result.hit = true;
                result.distance = t;
                result.point = ray.origin + t * ray.direction;
                result.normal = normal;
                result.color = color;
            }
        }
        return result;
    }
};

class Sphere {
public:
    vec3 center;
    float radius;
    vec3 color;

    Sphere(vec3 c, float r, vec3 col) : center(c), radius(r), color(col) {}

    Intersection intersect(const Ray& ray) const {
        Intersection result = { false, 0, vec3(0), vec3(0), vec3(0) };
        vec3 oc = ray.origin - center;
        float a = dot(ray.direction, ray.direction);
        float b = 2.0f * dot(oc, ray.direction);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant > 0) {
            float t = (-b - glm::sqrt(discriminant)) / (2.0f * a);
            if (t > 0) {
                result.hit = true;
                result.distance = t;
                result.point = ray.origin + t * ray.direction;
                result.normal = normalize(result.point - center);
                result.color = color;
            }
        }
        return result;
    }
};

// ��� Ŭ����
class Scene {
public:
    std::vector<Plane> planes;
    std::vector<Sphere> spheres;

    void addPlane(const Plane& plane) {
        planes.push_back(plane);
    }

    void addSphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

    Intersection trace(const Ray& ray, float tMin, float tMax) const {
        Intersection closestIntersection = { false, tMax, vec3(0), vec3(0), vec3(0) };
        for (const auto& plane : planes) {
            Intersection intersection = plane.intersect(ray);
            if (intersection.hit && intersection.distance > tMin && intersection.distance < closestIntersection.distance) {
                closestIntersection = intersection;
            }
        }
        for (const auto& sphere : spheres) {
            Intersection intersection = sphere.intersect(ray);
            if (intersection.hit && intersection.distance > tMin && intersection.distance < closestIntersection.distance) {
                closestIntersection = intersection;
            }
        }
        return closestIntersection;
    }
};

// ī�޶� Ŭ����
class Camera {
public:
    vec3 eye;
    vec3 u, v, w;
    float l, r, b, t, d;
    int nx, ny;

    Camera(vec3 e, vec3 u, vec3 v, vec3 w, float l, float r, float b, float t, float d, int nx, int ny)
        : eye(e), u(u), v(v), w(w), l(l), r(r), b(b), t(t), d(d), nx(nx), ny(ny) {
    }

    Ray getRay(int ix, int iy) const {
        float u_s = l + (r - l) * (ix + 0.5f) / nx;
        float v_s = b + (t - b) * (iy + 0.5f) / ny;
        vec3 direction = normalize(u_s * u + v_s * v - d * w);
        return { eye, direction };
    }
};

// ������ �Լ�
void render(const Scene& scene, const Camera& camera) {
    OutputImage.clear();
    for (int j = 0; j < ny; ++j) {
        for (int i = 0; i < nx; ++i) {
            Ray ray = camera.getRay(i, j);
            Intersection intersection = scene.trace(ray, 0.0f, std::numeric_limits<float>::max());
            vec3 color(0.0f, 0.0f, 0.0f); // �⺻ ����: ������
            if (intersection.hit) {
                color = vec3(1.0f, 1.0f, 1.0f); // ���� �� ���
            }
            OutputImage.push_back(color.r);
            OutputImage.push_back(color.g);
            OutputImage.push_back(color.b);
        }
    }
}

// OpenGL �ݹ� �Լ�
void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, static_cast<double>(Width), 0.0, static_cast<double>(Height), 1.0, -1.0);
}

// ���� �Լ�
int main(int argc, char* argv[]) {
    // GLFW �ʱ�ȭ
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // ������ ����
    GLFWwindow* window = glfwCreateWindow(Width, Height, "Ray Tracer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // ���ؽ�Ʈ ����
    glfwMakeContextCurrent(window);

    // GLEW �ʱ�ȭ
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    // OpenGL ���� ����
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // �������� �ݹ� ����
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // ��� ����
    Scene scene;
    scene.addPlane(Plane(vec3(0.0f, -2.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.5f, 0.5f, 0.5f)));
    scene.addSphere(Sphere(vec3(-4.0f, 0.0f, -7.0f), 1.0f, vec3(1.0f, 0.0f, 0.0f)));
    scene.addSphere(Sphere(vec3(0.0f, 0.0f, -7.0f), 2.0f, vec3(0.0f, 1.0f, 0.0f)));
    scene.addSphere(Sphere(vec3(4.0f, 0.0f, -7.0f), 1.0f, vec3(0.0f, 0.0f, 1.0f)));

    // ī�޶� ����
    Camera camera(eye, u, v, w, l, r, b, t, d, nx, ny);

    // ��� ������
    render(scene, camera);

    // ���� ����
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    // ����
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
