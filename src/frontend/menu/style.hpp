#pragma once
#include <imgui.h>

void SetupImGuiStyle()
{
	// SunBoy Dark style from ImThemes
	ImGuiStyle &style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 0.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 5.0f;
	style.ChildBorderSize = 3.0f;
	style.PopupRounding = 5.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	style.TabMinWidthForCloseButton = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05949641391634941f, 0.0f, 0.07296139001846313f, 0.9411764740943909f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.05882352963089943f, 0.0f, 0.07450980693101883f, 0.9411764740943909f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.47843137383461f, 0.1576009094715118f, 0.3085799515247345f, 0.5411764979362488f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.4000000059604645f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.6705882549285889f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.05882352963089943f, 0.0f, 0.07450980693101883f, 0.9411764740943909f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.47843137383461f, 0.1576009094715118f, 0.3085799515247345f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1560750305652618f, 0.06815376877784729f, 0.1587982773780823f, 0.9411764740943909f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.529411792755127f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.9803921580314636f, 0.2614378929138184f, 0.5997692942619324f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.8784313797950745f, 0.2411380112171173f, 0.5410407781600952f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.9803921580314636f, 0.2614378929138184f, 0.5997692942619324f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.4000000059604645f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.9764705896377563f, 0.05743944272398949f, 0.4899246692657471f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.3098039329051971f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.800000011920929f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.7490196228027344f, 0.09693194180727005f, 0.4037967026233673f, 0.7803921699523926f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.7490196228027344f, 0.09693194180727005f, 0.4037967026233673f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.6705882549285889f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.9490196108818054f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.5764706134796143f, 0.1763321757316589f, 0.3646326065063477f, 0.8627451062202454f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.800000011920929f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.6784313917160034f, 0.196878120303154f, 0.4234914183616638f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.4235294163227081f, 0.1328719705343246f, 0.2696519196033478f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 0.3490196168422699f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.9764705896377563f, 0.2603921294212341f, 0.5973702073097229f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}