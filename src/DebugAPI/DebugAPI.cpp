#pragma once
#include "DebugAPI.h"

#include <windows.h>

std::vector<DebugAPILine*> DebugAPI::LinesToDraw;

bool DebugAPI::CachedMenuData;

float DebugAPI::ScreenResX;
float DebugAPI::ScreenResY;

std::vector<std::string> DebugOverlayMenu::Hidden_Sources;

DebugAPILine::DebugAPILine(glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness, unsigned __int64 destroyTickCount)
{
	From = from;
	To = to;
	Color = color;
	fColor = DebugAPI::RGBToHex(color);
	Alpha = color.a * 100.0f;
	LineThickness = lineThickness;
	DestroyTickCount = destroyTickCount;
}

void DebugAPI::DrawLineForMS(const glm::vec3& from, const glm::vec3& to, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	DebugAPILine* oldLine = GetExistingLine(from, to, color, lineThickness);
	if (oldLine) {
		oldLine->From = from;
		oldLine->To = to;
		oldLine->DestroyTickCount = GetTickCount64() + liftetimeMS;
		oldLine->LineThickness = lineThickness;
		return;
	}

	DebugAPILine* newLine = new DebugAPILine(from, to, color, lineThickness, GetTickCount64() + liftetimeMS);
	LinesToDraw.push_back(newLine);
}

void DebugAPI::Update()
{
	auto hud = GetHUD();
	if (!hud || !hud->uiMovie)
		return;

	CacheMenuData();
	ClearLines2D(hud->uiMovie);

	for (int i = 0; i < LinesToDraw.size(); i++) {
		DebugAPILine* line = LinesToDraw[i];

		DrawLine3D(hud->uiMovie, line->From, line->To, line->fColor, line->LineThickness, line->Alpha);

		if (GetTickCount64() > line->DestroyTickCount) {
			LinesToDraw.erase(LinesToDraw.begin() + i);
			delete line;

			i--;
			continue;
		}
	}
}

void DebugAPI::DrawBoundsForMS(ObjectBound objectBound, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	auto boundRight = Util::GetBoundRightVectorRotated(objectBound);
	auto boundForward = Util::GetBoundForwardVectorRotated(objectBound);
	auto boundUp = Util::GetBoundUpVectorRotated(objectBound);

	auto objectLocation = objectBound.worldBoundMin;

	// x axis
	glm::vec3 glmXStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmXEnd1(glmXStart1.x + boundRight.x, glmXStart1.y + boundRight.y, glmXStart1.z + boundRight.z);
	// + y
	glm::vec3 glmXStart2(objectLocation.x + boundUp.x, objectLocation.y + boundUp.y, objectLocation.z + boundUp.z);
	glm::vec3 glmXEnd2(glmXStart2.x + boundRight.x, glmXStart2.y + boundRight.y, glmXStart2.z + boundRight.z);
	// + z
	glm::vec3 glmXStart3(objectLocation.x + boundForward.x, objectLocation.y + boundForward.y, objectLocation.z + boundForward.z);
	glm::vec3 glmXEnd3(glmXStart3.x + boundRight.x, glmXStart3.y + boundRight.y, glmXStart3.z + boundRight.z);
	// + y + z
	glm::vec3 glmXStart4(objectLocation.x + boundUp.x + boundForward.x, objectLocation.y + boundUp.y + boundForward.y, objectLocation.z + boundUp.z + boundForward.z);
	glm::vec3 glmXEnd4(glmXStart4.x + boundRight.x, glmXStart4.y + boundRight.y, glmXStart4.z + boundRight.z);

	// y axis
	glm::vec3 glmYStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmYEnd1(glmYStart1.x + boundForward.x, glmYStart1.y + boundForward.y, glmYStart1.z + boundForward.z);
	// + z
	glm::vec3 glmYStart2(objectLocation.x + boundUp.x, objectLocation.y + boundUp.y, objectLocation.z + boundUp.z);
	glm::vec3 glmYEnd2(glmYStart2.x + boundForward.x, glmYStart2.y + boundForward.y, glmYStart2.z + boundForward.z);
	// + x
	glm::vec3 glmYStart3(objectLocation.x + boundRight.x, objectLocation.y + boundRight.y, objectLocation.z + boundRight.z);
	glm::vec3 glmYEnd3(glmYStart3.x + boundForward.x, glmYStart3.y + boundForward.y, glmYStart3.z + boundForward.z);
	// + z + x
	glm::vec3 glmYStart4(objectLocation.x + boundUp.x + boundRight.x, objectLocation.y + boundUp.y + boundRight.y, objectLocation.z + boundUp.z + boundRight.z);
	glm::vec3 glmYEnd4(glmYStart4.x + boundForward.x, glmYStart4.y + boundForward.y, glmYStart4.z + boundForward.z);

	// z axis
	glm::vec3 glmZStart1(objectLocation.x, objectLocation.y, objectLocation.z);
	glm::vec3 glmZEnd1(glmZStart1.x + boundUp.x, glmZStart1.y + boundUp.y, glmZStart1.z + boundUp.z);
	// + x
	glm::vec3 glmZStart2(objectLocation.x + boundRight.x, objectLocation.y + boundRight.y, objectLocation.z + boundRight.z);
	glm::vec3 glmZEnd2(glmZStart2.x + boundUp.x, glmZStart2.y + boundUp.y, glmZStart2.z + boundUp.z);
	// + y
	glm::vec3 glmZStart3(objectLocation.x + boundForward.x, objectLocation.y + boundForward.y, objectLocation.z + boundForward.z);
	glm::vec3 glmZEnd3(glmZStart3.x + boundUp.x, glmZStart3.y + boundUp.y, glmZStart3.z + boundUp.z);
	// + x + y
	glm::vec3 glmZStart4(objectLocation.x + boundRight.x + boundForward.x, objectLocation.y + boundRight.y + boundForward.y, objectLocation.z + boundRight.z + boundForward.z);
	glm::vec3 glmZEnd4(glmZStart4.x + boundUp.x, glmZStart4.y + boundUp.y, glmZStart4.z + boundUp.z);

	DebugAPI::DrawLineForMS(glmZStart1, glmZEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart2, glmZEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart3, glmZEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmZStart4, glmZEnd4, liftetimeMS, color, lineThickness);

	DebugAPI::DrawLineForMS(glmYStart1, glmYEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart2, glmYEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart3, glmYEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmYStart4, glmYEnd4, liftetimeMS, color, lineThickness);

	DebugAPI::DrawLineForMS(glmXStart1, glmXEnd1, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart2, glmXEnd2, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart3, glmXEnd3, liftetimeMS, color, lineThickness);
	DebugAPI::DrawLineForMS(glmXStart4, glmXEnd4, liftetimeMS, color, lineThickness);
}

void DebugAPI::DrawSphere(glm::vec3 origin, float radius, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	DrawCircle(origin, radius, glm::vec3(0.0f, 0.0f, 0.0f), liftetimeMS, color, lineThickness);
	DrawCircle(origin, radius, glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f), liftetimeMS, color, lineThickness);
}

void DebugAPI::DrawCircle(glm::vec3 origin, float radius, glm::vec3 eulerAngles, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	glm::vec3 lastEndPos = Util::GetPointOnRotatedCircle(origin, radius, CIRCLE_NUM_SEGMENTS, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

	for (int i = 0; i <= CIRCLE_NUM_SEGMENTS; i++) {
		glm::vec3 currEndPos = Util::GetPointOnRotatedCircle(origin, radius, (float)i, (float)(CIRCLE_NUM_SEGMENTS - 1), eulerAngles);

		DrawLineForMS(
			lastEndPos,
			currEndPos,
			liftetimeMS,
			color,
			lineThickness);

		lastEndPos = currEndPos;
	}
}

void DebugAPI::DrawArc(const RE::NiPoint3& origin, float radius, float startAngle, float endAngle, const RE::NiMatrix3& matrix, int liftetimeMS, const glm::vec4& color, float lineThickness)
{
	if (std::abs(radius) < 1e-6)
		return;

	float radian = startAngle < endAngle ? endAngle - startAngle : endAngle - startAngle + glm::two_pi<float>();

	auto GetPointOnArc = [](const RE::NiPoint3& origin, float radius, float startAngle, float radian, float i, float maxI, const RE::NiMatrix3& matrix) -> RE::NiPoint3 {
		auto currentAngle = startAngle + radian * (i / maxI);
		return origin + matrix * RE::NiPoint3(radius * std::sinf(currentAngle), radius * std::cosf(currentAngle), 0.f);
	};

	auto currentPoint = GetPointOnArc(origin, radius, startAngle, radian, 0, CIRCLE_NUM_SEGMENTS, matrix);
	DrawLineForMS(
		Util::NiPointToVec(currentPoint),
		Util::NiPointToVec(origin),
		liftetimeMS,
		color,
		lineThickness);

	for (int i = 1; i <= CIRCLE_NUM_SEGMENTS; i++) {
		auto nextPoint = GetPointOnArc(origin, radius, startAngle, radian, i, CIRCLE_NUM_SEGMENTS, matrix);

		DrawLineForMS(
			Util::NiPointToVec(currentPoint),
			Util::NiPointToVec(nextPoint),
			liftetimeMS,
			color,
			lineThickness);

		currentPoint = nextPoint;
	}

	DrawLineForMS(
		Util::NiPointToVec(currentPoint),
		Util::NiPointToVec(origin),
		liftetimeMS,
		color,
		lineThickness);
}

DebugAPILine* DebugAPI::GetExistingLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color, float lineThickness)
{
	for (int i = 0; i < LinesToDraw.size(); i++) {
		DebugAPILine* line = LinesToDraw[i];

		if (
			Util::IsRoughlyEqual(from.x, line->From.x, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(from.y, line->From.y, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(from.z, line->From.z, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(to.x, line->To.x, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(to.y, line->To.y, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(to.z, line->To.z, DRAW_LOC_MAX_DIF) &&
			Util::IsRoughlyEqual(lineThickness, line->LineThickness, DRAW_LOC_MAX_DIF) &&
			color == line->Color) {
			return line;
		}
	}

	return nullptr;
}

void DebugAPI::DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, float color, float lineThickness, float alpha)
{
	if (Util::IsPosBehindPlayerCamera(from) && Util::IsPosBehindPlayerCamera(to))
		return;

	glm::vec2 screenLocFrom = WorldToScreenLoc(movie, from);
	glm::vec2 screenLocTo = WorldToScreenLoc(movie, to);

	DrawLine2D(movie, screenLocFrom, screenLocTo, color, lineThickness, alpha);
}

void DebugAPI::DrawLine3D(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 from, glm::vec3 to, glm::vec4 color, float lineThickness)
{
	DrawLine3D(movie, from, to, RGBToHex(glm::vec3(color.r, color.g, color.b)), lineThickness, color.a * 100.0f);
}

void DebugAPI::DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, float color, float lineThickness, float alpha)
{
	// all parts of the line are off screen - don't need to draw them
	if (!IsOnScreen(from, to))
		return;

	FastClampToScreen(from);
	FastClampToScreen(to);

	// lineStyle(thickness:Number = NaN, color : uint = 0, alpha : Number = 1.0, pixelHinting : Boolean = false,
	// scaleMode : String = "normal", caps : String = null, joints : String = null, miterLimit : Number = 3) :void
	//
	// CapsStyle values: 'NONE', 'ROUND', 'SQUARE'
	// const char* capsStyle = "NONE";

	RE::GFxValue argsLineStyle[3]{ lineThickness, color, alpha };
	movie->Invoke("lineStyle", nullptr, argsLineStyle, 3);

	RE::GFxValue argsStartPos[2]{ from.x, from.y };
	movie->Invoke("moveTo", nullptr, argsStartPos, 2);

	RE::GFxValue argsEndPos[2]{ to.x, to.y };
	movie->Invoke("lineTo", nullptr, argsEndPos, 2);

	movie->Invoke("endFill", nullptr, nullptr, 0);
}

void DebugAPI::DrawLine2D(RE::GPtr<RE::GFxMovieView> movie, glm::vec2 from, glm::vec2 to, glm::vec4 color, float lineThickness)
{
	DrawLine2D(movie, from, to, RGBToHex(glm::vec3(color.r, color.g, color.b)), lineThickness, color.a * 100.0f);
}

void DebugAPI::ClearLines2D(RE::GPtr<RE::GFxMovieView> movie)
{
	movie->Invoke("clear", nullptr, nullptr, 0);
}

RE::GPtr<RE::IMenu> DebugAPI::GetHUD()
{
	RE::GPtr<RE::IMenu> hud = RE::UI::GetSingleton()->GetMenu(DebugOverlayMenu::MENU_NAME);
	return hud;
}

float DebugAPI::RGBToHex(glm::vec3 rgb)
{
	return ConvertComponentR(rgb.r * 255) + ConvertComponentG(rgb.g * 255) + ConvertComponentB(rgb.b * 255);
}

// if drawing outside the screen rect, at some point Scaleform seems to start resizing the rect internally, without
// increasing resolution. This will cause all line draws to become more and more pixelated and increase in thickness
// the farther off screen even one line draw goes. I'm allowing some leeway, then I just clamp the
// coordinates to the screen rect.
//
// this is inaccurate. A more accurate solution would require finding the sub vector that overshoots the screen rect between
// two points and scale the vector accordingly. Might implement that at some point, but the inaccuracy is barely noticeable
const float CLAMP_MAX_OVERSHOOT = 10000.0f;
void DebugAPI::FastClampToScreen(glm::vec2& point)
{
	if (point.x < 0.0) {
		float overshootX = abs(point.x);
		if (overshootX > CLAMP_MAX_OVERSHOOT)
			point.x += overshootX - CLAMP_MAX_OVERSHOOT;
	} else if (point.x > ScreenResX) {
		float overshootX = point.x - ScreenResX;
		if (overshootX > CLAMP_MAX_OVERSHOOT)
			point.x -= overshootX - CLAMP_MAX_OVERSHOOT;
	}

	if (point.y < 0.0) {
		float overshootY = abs(point.y);
		if (overshootY > CLAMP_MAX_OVERSHOOT)
			point.y += overshootY - CLAMP_MAX_OVERSHOOT;
	} else if (point.y > ScreenResY) {
		float overshootY = point.y - ScreenResY;
		if (overshootY > CLAMP_MAX_OVERSHOOT)
			point.y -= overshootY - CLAMP_MAX_OVERSHOOT;
	}
}

float DebugAPI::ConvertComponentR(float value)
{
	return (value * 0xffff) + value;
}

float DebugAPI::ConvertComponentG(float value)
{
	return (value * 0xff) + value;
}

float DebugAPI::ConvertComponentB(float value)
{
	return value;
}

glm::vec2 DebugAPI::WorldToScreenLoc(RE::GPtr<RE::GFxMovieView> movie, glm::vec3 worldLoc)
{
	glm::vec2 screenLocOut;
	RE::NiPoint3 niWorldLoc(worldLoc.x, worldLoc.y, worldLoc.z);

	float zVal;

#if ANNIVERSARY_EDITION
	uintptr_t WorldToCamMatrix = REL::ID(406126).address(), ViewPort = REL::ID(406160).address();  //Anniversary Edition
#else
	uintptr_t WorldToCamMatrix = REL::ID(519579).address(), ViewPort = REL::ID(519618).address();  //Special Edition
#endif

	RE::NiCamera::WorldPtToScreenPt3((float(*)[4])(WorldToCamMatrix), *((RE::NiRect<float>*)ViewPort), niWorldLoc, screenLocOut.x, screenLocOut.y, zVal, 1e-5f);
	RE::GRectF rect = movie->GetVisibleFrameRect();

	screenLocOut.x = rect.left + (rect.right - rect.left) * screenLocOut.x;
	screenLocOut.y = 1.0f - screenLocOut.y;  // Flip y for Flash coordinate system
	screenLocOut.y = rect.top + (rect.bottom - rect.top) * screenLocOut.y;

	return screenLocOut;
}

DebugOverlayMenu::DebugOverlayMenu()
{
	auto scaleformManager = RE::BSScaleformManager::GetSingleton();
	if (!scaleformManager) {
		ERROR("BTPS: failed to initialize DebugOverlayMenu - ScaleformManager not found");
		return;
	}

	depthPriority = 0;

	menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
	menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);
	menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);

	inputContext = Context::kNone;

	scaleformManager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* a_def) -> void {
		a_def->SetState(RE::GFxState::StateType::kLog,
			RE::make_gptr<Logger>().get());
	});
}

void DebugOverlayMenu::Register()
{
	INFO("BTPS: registering DebugOverlayMenu...");

	auto ui = RE::UI::GetSingleton();
	if (ui) {
		ui->Register(MENU_NAME, Creator);
		DebugOverlayMenu::Load();

		INFO("BTPS: successfully registered DebugOverlayMenu");
	} else
		ERROR("BTPS: failed to register DebugOverlayMenu");
}

void DebugOverlayMenu::Load()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	} else
		logger::warn("BTPS: failed to show DebugOverlayMenu");
}

void DebugOverlayMenu::Unload()
{
	auto msgQ = RE::UIMessageQueue::GetSingleton();
	if (msgQ) {
		msgQ->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
	} else
		logger::warn("BTPS: failed to hide DebugOverlayMenu");
}

void DebugOverlayMenu::Show(std::string source)
{
	auto sourceIdx = std::find(Hidden_Sources.begin(), Hidden_Sources.end(), source);
	if (sourceIdx != Hidden_Sources.end())
		Hidden_Sources.erase(sourceIdx);

	if (Hidden_Sources.empty())
		ToggleVisibility(true);
}

void DebugOverlayMenu::Hide(std::string source)
{
	auto sourceIdx = std::find(Hidden_Sources.begin(), Hidden_Sources.end(), source);
	if (sourceIdx == Hidden_Sources.end())
		Hidden_Sources.push_back(source);

	if (!Hidden_Sources.empty())
		ToggleVisibility(false);
}

void DebugOverlayMenu::ToggleVisibility(bool mode)
{
	auto ui = RE::UI::GetSingleton();
	if (!ui)
		return;

	auto overlayMenu = ui->GetMenu(DebugOverlayMenu::MENU_NAME);
	if (!overlayMenu || !overlayMenu->uiMovie)
		return;

	overlayMenu->uiMovie->SetVisible(mode);
}

void DebugAPI::CacheMenuData()
{
	if (CachedMenuData)
		return;

	RE::GPtr<RE::IMenu> menu = RE::UI::GetSingleton()->GetMenu(DebugOverlayMenu::MENU_NAME);
	if (!menu || !menu->uiMovie)
		return;

	RE::GRectF rect = menu->uiMovie->GetVisibleFrameRect();

	ScreenResX = abs(rect.left - rect.right);
	ScreenResY = abs(rect.top - rect.bottom);

	CachedMenuData = true;
	INFO("BTPS: DebugAPI::CacheMenuData");
}

// void DebugAPI::ClampVectorToScreen(glm::vec2& from, glm::vec2& to)
// {
// 	glm::vec2 between = to - from;
// 	float m = to.y / from.y;
//
// 	float angle = glm::atan(m);
//
// 	ClampPointToScreen(from, angle);
// 	ClampPointToScreen(to, angle);
// }
//
// void DebugAPI::ClampPointToScreen(glm::vec2& point, float lineAngle)
// {
// 	if (point.y < 0.0)
// 	{
// 		float overshootY = point.y;
// 		float overshootX = glm::tan(lineAngle) * overshootY;
//
// 		point.y += overshootY;
// 		point.x += overshootX;
// 	}
// 	else if (point.y > ScreenResY)
// 	{
// 		float overshootY = point.y - ScreenResY;
// 		float overshootX = glm::tan(lineAngle) * overshootY;
//
// 		point.y -= overshootY;
// 		point.x -= overshootX;
// 	}
//
// 	if (point.x < 0.0)
// 	{
// 		float overshootX = point.x;
// 		float overshootY = glm::tan(lineAngle) * overshootX;
//
// 		point.y += overshootY;
// 		point.x += overshootX;
// 	}
// 	else if (point.x > ScreenResX)
// 	{
// 		float overshootX = point.x - ScreenResX;
// 		float overshootY = glm::tan(lineAngle) * overshootX;
//
// 		point.y -= overshootY;
// 		point.x -= overshootX;
// 	}
// }

bool DebugAPI::IsOnScreen(glm::vec2 from, glm::vec2 to)
{
	return IsOnScreen(from) || IsOnScreen(to);
}

bool DebugAPI::IsOnScreen(glm::vec2 point)
{
	return (point.x <= ScreenResX && point.x >= 0.0 && point.y <= ScreenResY && point.y >= 0.0);
}

void DebugOverlayMenu::AdvanceMovie(float a_interval, std::uint32_t a_currentTime)
{
	RE::IMenu::AdvanceMovie(a_interval, a_currentTime);

	DebugAPI::Update();
}
