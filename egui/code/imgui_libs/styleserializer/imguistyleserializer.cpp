/**

  @file    imguistyleserializer.cpp
  @brief   Save/load Dear ImGui style.
  @author  Many authors, MIT license
  @version 1.0
  @date    26.4.2021

****************************************************************************************************
*/
#include "imguistyleserializer.h"
#include "imgui_internal.h"

#include <string.h>
#include <stdio.h>


void ImGuiSetuptyle(bool bStyleDark, float alpha)
{
    ImGuiStyle& style = ImGui::GetStyle();

    // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha = 1.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.50f, 0.50f, 0.50f, 0.14f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.44f, 0.96f, 0.96f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.96f, 0.74f, 0.44f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Separator]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    style.Colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    style.Colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);

    style.Colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.68f, 0.68f, 0.68f, 0.75f);
    style.Colors[ImGuiCol_TableBorderLight]      = ImVec4(0.68f, 0.68f, 0.68f, 0.50f);

    style.Colors[ImGuiCol_Tab]                   = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
    style.Colors[ImGuiCol_TabHovered]            = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TabActive]             = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.77f, 0.77f, 0.77f, 0.86f);
    style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

    style.WindowPadding = ImVec2(8, 8);
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;

    if (bStyleDark)
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );

            if( S < 0.1f )
            {
               V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
            if( col.w < 1.00f )
            {
                col.w *= alpha;
            }
        }
    }
    else
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            if( col.w < 1.00f )
            {
                col.x *= alpha;
                col.y *= alpha;
                col.z *= alpha;
                col.w *= alpha;
            }
        }
    }

}


bool ImGuiSaveStyle(const char* filename,const ImGuiStyle& style)
{
    FILE* f = fopen(filename, "wt");
    if (!f)  return false;

    ImGuiIO &io = ImGui::GetIO();
    fprintf(f, "[GlobalScale]\n%1.3f\n", io.FontGlobalScale);
    fprintf(f, "[Alpha]\n%1.3f\n", style.Alpha);
    fprintf(f, "[WindowPadding]\n%1.3f %1.3f\n", style.WindowPadding.x,style.WindowPadding.y);
    fprintf(f, "[WindowMinSize]\n%1.3f %1.3f\n", style.WindowMinSize.x,style.WindowMinSize.y);
    fprintf(f, "[FramePadding]\n%1.3f %1.3f\n", style.FramePadding.x,style.FramePadding.y);
    fprintf(f, "[FrameRounding]\n%1.3f\n", style.FrameRounding);
    fprintf(f, "[ItemSpacing]\n%1.3f %1.3f\n", style.ItemSpacing.x,style.ItemSpacing.y);
    fprintf(f, "[ItemInnerSpacing]\n%1.3f %1.3f\n", style.ItemInnerSpacing.x,style.ItemInnerSpacing.y);
    fprintf(f, "[TouchExtraPadding]\n%1.3f %1.3f\n", style.TouchExtraPadding.x,style.TouchExtraPadding.y);
    fprintf(f, "[WindowRounding]\n%1.3f\n", style.WindowRounding);
    fprintf(f, "[IndentSpacing]\n%1.3f\n", style.IndentSpacing);
    fprintf(f, "[ColumnsMinSpacing]\n%1.3f\n", style.ColumnsMinSpacing);
    fprintf(f, "[ScrollbarSize]\n%1.3f\n", style.ScrollbarSize);

    for (size_t i = 0; i != ImGuiCol_COUNT; i++)
    {
        const ImVec4& c = style.Colors[i];
        fprintf(f, "[%s]\n", ImGui::GetStyleColorName((ImGuiCol)i));
        fprintf(f, "%1.3f %1.3f %1.3f %1.3f\n",c.x,c.y,c.z,c.w);
    }

    fprintf(f,"\n");
    fclose(f);

    return true;
}

bool ImGuiLoadStyle(const char* filename,ImGuiStyle& style)
{
    // Load .style file
    if (!filename)  return false;

    // Load file into memory
    FILE* f;
    if ((f = fopen(filename, "rt")) == NULL) return false;
    if (fseek(f, 0, SEEK_END))  {
       fclose(f);
       return false;
    }
    const long f_size_signed = ftell(f);
    if (f_size_signed == -1)    {
       fclose(f);
       return false;
    }
    size_t f_size = (size_t)f_size_signed;
    if (fseek(f, 0, SEEK_SET))  {
       fclose(f);
       return false;
    }
    char* f_data = (char*)ImGui::MemAlloc(f_size+1);
    f_size = fread(f_data, 1, f_size, f); // Text conversion alter read size so let's not be fussy about return value
    fclose(f);
    if (f_size == 0)    {
        ImGui::MemFree(f_data);
        return false;
    }
    f_data[f_size] = 0;

    // Parse file in memory
    char name[128];name[0]='\0';
    const char* buf_end = f_data + f_size;
    for (const char* line_start = f_data; line_start < buf_end; )
    {
        const char* line_end = line_start;
        while (line_end < buf_end && *line_end != '\n' && *line_end != '\r')
            line_end++;

        if (name[0]=='\0' && line_start[0] == '[' && line_end > line_start && line_end[-1] == ']')
        {
            ImFormatString(name, IM_ARRAYSIZE(name), "%.*s", (int)(line_end-line_start-2), line_start+1);
            //fprintf(stderr,"name: %s\n",name);  // dbg
        }
        else if (name[0]!='\0')
        {

            float *pf[4]= {0,0,0,0};
            int npf = 0;

            // parsing 'name' here by filling the fields above
            {
                if (strcmp(name, "GlobalScale")==0) {
                    npf=1;
                    ImGuiIO &io = ImGui::GetIO();
                    pf[0]=&io.FontGlobalScale;
                }
                else if (strcmp(name, "Alpha")==0)                     {npf=1;pf[0]=&style.Alpha;}
                else if (strcmp(name, "WindowPadding")==0)             {npf=2;pf[0]=&style.WindowPadding.x;pf[1]=&style.WindowPadding.y;}
                else if (strcmp(name, "WindowMinSize")==0)             {npf=2;pf[0]=&style.WindowMinSize.x;pf[1]=&style.WindowMinSize.y;}
                else if (strcmp(name, "FramePadding")==0)              {npf=2;pf[0]=&style.FramePadding.x;pf[1]=&style.FramePadding.y;}
                else if (strcmp(name, "FrameRounding")==0)             {npf=1;pf[0]=&style.FrameRounding;}
                else if (strcmp(name, "ItemSpacing")==0)               {npf=2;pf[0]=&style.ItemSpacing.x;pf[1]=&style.ItemSpacing.y;}
                else if (strcmp(name, "ItemInnerSpacing")==0)          {npf=2;pf[0]=&style.ItemInnerSpacing.x;pf[1]=&style.ItemInnerSpacing.y;}
                else if (strcmp(name, "TouchExtraPadding")==0)         {npf=2;pf[0]=&style.TouchExtraPadding.x;pf[1]=&style.TouchExtraPadding.y;}
                else if (strcmp(name, "WindowRounding")==0)            {npf=1;pf[0]=&style.WindowRounding;}
                else if (strcmp(name, "IndentSpacing")==0)             {npf=1;pf[0]=&style.IndentSpacing;}
                else if (strcmp(name, "ColumnsMinSpacing")==0)         {npf=1;pf[0]=&style.ColumnsMinSpacing;}
                else if (strcmp(name, "ScrollbarSize")==0)            {npf=1;pf[0]=&style.ScrollbarSize;}
                // all the colors here
                else {
                    for (int j=0;j<ImGuiCol_COUNT;j++)    {
                        if (strcmp(name,ImGui::GetStyleColorName(j))==0)    {
                            npf = 4;
                            ImVec4& color = style.Colors[j];
                            pf[0]=&color.x;pf[1]=&color.y;pf[2]=&color.z;pf[3]=&color.w;
                            break;
                        }
                    }
                }
            }

            //fprintf(stderr,"name: %s npf=%d\n",name,npf);  // dbg
            // parsing values here and filling pf[]
            float x,y,z,w;
            switch (npf)	{
                case 1:
                if (sscanf(line_start, "%f", &x) == npf)	{
                    *pf[0] = x;
                }
                else fprintf(stderr,"Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n",filename,name);
                break;
                case 2:
                if (sscanf(line_start, "%f %f", &x, &y) == npf)	{
                    *pf[0] = x;*pf[1] = y;
                }
                else fprintf(stderr,"Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n",filename,name);
                break;
                case 3:
                if (sscanf(line_start, "%f %f %f", &x, &y, &z) == npf)	{
                    *pf[0] = x;*pf[1] = y;*pf[2] = z;
                }
                else fprintf(stderr,"Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n",filename,name);
                break;
                case 4:
                if (sscanf(line_start, "%f %f %f %f", &x, &y, &z, &w) == npf)	{
                    *pf[0] = x;*pf[1] = y;*pf[2] = z;*pf[3] = w;
                }
                else fprintf(stderr,"Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (parsing error).\n",filename,name);
                break;
                default:
                fprintf(stderr,"Warning in ImGui::LoadStyle(\"%s\"): skipped [%s] (unknown field).\n",filename,name);
                break;
            }
            /*
            // Same reference code from <imgui.cpp> to help parsing
            float x, y;
            int i;
            if (sscanf(line_start, "Pos=%f,%f", &x, &y) == 2)
                settings->Pos = ImVec2(x, y);
            else if (sscanf(line_start, "Size=%f,%f", &x, &y) == 2)
                settings->Size = ImMax(ImVec2(x, y), g.Style.WindowMinSize);
            else if (sscanf(line_start, "Collapsed=%d", &i) == 1)
                settings->Collapsed = (i != 0);
            */
            //---------------------------------------------------------------------------------
            name[0]='\0'; // mandatory
        }

        line_start = line_end+1;
    }

    // Release memory
    ImGui::MemFree(f_data);
    return true;
}
