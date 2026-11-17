// ============================================================================
//  Простой графический калькулятор на C++17 (Dear ImGui + GLFW + OpenGL3)
//
//  Интерфейс:
//    - Окно фиксированного размера (нельзя изменить пользователем)
//    - Два числовых поля ввода: "Число А" и "Число Б"
//    - Четыре кнопки в ряд: "+", "-", "*", "/"
//    - Результат выводится крупным текстом; при делении на ноль показывается
//      красное сообщение "Ошибка: деление на ноль!"
// ============================================================================

#include "imgui.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include <cstdio>

// ----------------------------------------------------------------------------
// Состояние калькулятора
// ----------------------------------------------------------------------------
struct CalculatorState
{
    float valueA       = 0.0f;   // "Число А"
    float valueB       = 0.0f;   // "Число Б"
    float result       = 0.0f;   // результат операции
    bool  hasResult    = false;  // была ли выполнена хотя бы одна операция
    bool  divideByZero = false;  // флаг ошибки "деление на ноль"

    ImFont* bigFont    = nullptr;  // шрифт для крупного вывода результата
};

// Мгновенно выполняет выбранную операцию и записывает результат в состояние.
static void PerformOperation(CalculatorState& st, char op)
{
    st.divideByZero = false;

    switch (op)
    {
        case '+': st.result = st.valueA + st.valueB; break;
        case '-': st.result = st.valueA - st.valueB; break;
        case '*': st.result = st.valueA * st.valueB; break;
        case '/':
            if (st.valueB == 0.0f)
            {
                st.divideByZero = true;      // проверка на деление на ноль
                st.result       = 0.0f;
            }
            else
            {
                st.result = st.valueA / st.valueB;
            }
            break;
        default: return;
    }

    st.hasResult = true;
}

// ----------------------------------------------------------------------------
// Отрисовка ImGui-окна калькулятора
// ----------------------------------------------------------------------------
static void DrawCalculatorWindow(CalculatorState& st)
{
    // Фиксированный размер окна интерфейса: без ресайза, перемещения и сворачивания
    constexpr ImGuiWindowFlags winFlags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove   |
        ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowSize(ImVec2(420.0f, 260.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Always);

    ImGui::Begin("Калькулятор", nullptr, winFlags);

    // --- Два числовых поля ввода ---
    ImGui::InputFloat("Число А", &st.valueA, 0.0f, 0.0f, "%.6g");
    ImGui::InputFloat("Число Б", &st.valueB, 0.0f, 0.0f, "%.6g");

    ImGui::Spacing();

    // --- Четыре кнопки операций в один ряд ---
    const char* ops[4] = { "+", "-", "*", "/" };
    for (int i = 0; i < 4; ++i)
    {
        if (i > 0)
            ImGui::SameLine();

        if (ImGui::Button(ops[i], ImVec2(85.0f, 40.0f)))
            PerformOperation(st, ops[i][0]);
    }

    ImGui::Separator();
    ImGui::Spacing();

    // --- Вывод результата ---
    if (st.divideByZero)
    {
        // Ошибка — красным цветом
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
        if (st.bigFont != nullptr)
            ImGui::PushFont(st.bigFont);
        ImGui::TextUnformatted("Ошибка: деление на ноль!");
        if (st.bigFont != nullptr)
            ImGui::PopFont();
        ImGui::PopStyleColor();
    }
    else if (st.hasResult)
    {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Результат: %g", static_cast<double>(st.result));

        // Крупный текст
        if (st.bigFont != nullptr)
            ImGui::PushFont(st.bigFont);
        ImGui::TextUnformatted(buf);
        if (st.bigFont != nullptr)
            ImGui::PopFont();
    }
    else
    {
        ImGui::TextDisabled("Введите числа и нажмите операцию");
    }

    ImGui::End();
}

// ----------------------------------------------------------------------------
// main
// ----------------------------------------------------------------------------
int main(int, char**)
{
    // --- Инициализация GLFW ---
    if (!glfwInit())
        return 1;

    // Версия OpenGL-контекста (3.0+ достаточно для бэкенда imgui_impl_opengl3)
#ifdef __APPLE__
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // Окно фиксированного размера: пользователь не может его изменить
    GLFWwindow* window = glfwCreateWindow(
        460, 300,
        "Калькулятор - ImGui + GLFW",
        nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwSetWindowAttrib(window, GLFW_RESIZABLE, GLFW_FALSE);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // вертикальная синхронизация

    // --- Инициализация Dear ImGui ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    CalculatorState state;

    // Встроенный шрифт ProggyClean содержит только ASCII, поэтому для
    // корректного отображения кириллицы подгружаем системный TTF-шрифт:
    //   Windows (MinGW из Code::Blocks) — Arial;
    //   Linux — DejaVuSans.
    // Если файл не найден, остаётся стандартный шрифт ImGui.
#ifdef _WIN32
    const char* fontPath = "C:\\Windows\\Fonts\\arial.ttf";
#else
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    ImFont* regular = io.Fonts->AddFontFromFileTTF(
        fontPath, 18.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (regular != nullptr)
    {
        // Тот же шрифт, но в 2 раза крупнее — для вывода результата
        state.bigFont = io.Fonts->AddFontFromFileTTF(
            fontPath, 36.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // --- Главный цикл ---
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        DrawCalculatorWindow(state);

        ImGui::Render();

        int displayW = 0, displayH = 0;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // --- Завершение работы ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
