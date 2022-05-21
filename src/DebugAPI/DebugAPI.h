#include "Util.h"

class DebugAPILine
{
public:
	DebugAPILine(glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness, unsigned __int64 destroyTickCount);

	glm::vec3 From;
	glm::vec3 To;
	glm::vec4 Color;
	float fColor;
	float Alpha;
	float LineThickness;

	unsigned __int64 DestroyTickCount;
};

class DebugAPI
{
public:
	static void Update();

	static RE::GPtr<RE::IMenu> GetHUD();

	static void DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, float color, float lineThickness, float alpha);
	static void DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, glm::vec4 color, float lineThickness);
	static void DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, float color, float lineThickness, float alpha);
	static void DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness);
	static void ClearLines2D(RE::GPtr<RE::GFxMovieView> movie);

	static void DrawLineForMS(const glm::vec3& from, const glm::vec3& to, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
	static void DrawBoundsForMS(ObjectBound objectBound, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
	static void DrawSphere(glm::vec3, float radius, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
	static void DrawCircle(glm::vec3, float radius, glm::vec3 eulerAngles, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);
	static void DrawArc(const RE::NiPoint3& origin, float radius, float startAngle, float endAngle, const RE::NiMatrix3& matrix, int liftetimeMS = 10, const glm::vec4& color = { 1.0f, 0.0f, 0.0f, 1.0f }, float lineThickness = 1);

	static std::vector<DebugAPILine*> LinesToDraw;

	static bool DEBUG_API_REGISTERED;

	static constexpr int CIRCLE_NUM_SEGMENTS = 50;

	static constexpr float DRAW_LOC_MAX_DIF = 1.0f;

	static glm::vec2 WorldToScreenLoc(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 worldLoc);
	static float RGBToHex(glm::vec3 rgb);

	static void FastClampToScreen(glm::vec2& point);

	// 	static void ClampVectorToScreen(glm::vec2& from, glm::vec2& to);
	// 	static void ClampPointToScreen(glm::vec2& point, float lineAngle);

	static bool IsOnScreen(glm::vec2 from, glm::vec2 to);
	static bool IsOnScreen(glm::vec2 point);

	static void CacheMenuData();

	static bool CachedMenuData;

	static float ScreenResX;
	static float ScreenResY;

private:
	static float ConvertComponentR(float value);
	static float ConvertComponentG(float value);
	static float ConvertComponentB(float value);
	// returns true if there is already a line with the same color at around the same from and to position
	// with some leniency to bundle together lines in roughly the same spot (see DRAW_LOC_MAX_DIF)
	static DebugAPILine* GetExistingLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float lineThickness);
};

class DebugOverlayMenu : RE::IMenu
{
public:
	static constexpr const char* MENU_PATH = "BetterThirdPersonSelection/BTPS_overlay_menu";
	static constexpr const char* MENU_NAME = "BTPS Ovelay Menu";

	DebugOverlayMenu();

	static void Register();

	static std::vector<std::string> Hidden_Sources;

	static void Load();
	static void Unload();

	static void Show(std::string source);
	static void Hide(std::string source);
	static void ToggleVisibility(bool mode);

	static RE::stl::owner<RE::IMenu*> Creator() { return new DebugOverlayMenu(); }

	void AdvanceMovie(float a_interval, std::uint32_t a_currentTime) override;

private:
	class Logger : public RE::GFxLog
	{
	public:
		void LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList) override
		{
			std::string fmt(a_fmt ? a_fmt : "");
			while (!fmt.empty() && fmt.back() == '\n') {
				fmt.pop_back();
			}

			std::va_list args;
			va_copy(args, a_argList);
			std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
			std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
			va_end(args);

			logger::info("{}"sv, buf.data());
		}
	};
};
