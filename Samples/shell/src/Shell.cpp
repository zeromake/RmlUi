/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "../include/Shell.h"
#include "../include/PlatformExtensions.h"
#include "../include/ShellFileInterface.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Debugger.h>
#include <filesystem>
#ifdef _WIN32
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif

static Rml::UniquePtr<ShellFileInterface> file_interface;

bool Shell::Initialize()
{
	// Find the path to the 'Samples' directory.
	Rml::String root = PlatformExtensions::FindSamplesRoot();
	if (root.empty())
		return false;

	// The shell overrides the default file interface so that absolute paths in RML/RCSS-documents are relative to the 'Samples' directory.
	file_interface = Rml::MakeUnique<ShellFileInterface>(root);
	Rml::SetFileInterface(file_interface.get());

	return true;
}

void Shell::Shutdown()
{
	file_interface.reset();
}

const static Rml::Array<Rml::String, 3> fontExts = {
    ".ttc",
    ".ttf",
    ".otf",
};

static void loadFontsExist(Rml::Vector<Rml::String> fontPaths) {
	for (auto &fontPath: fontPaths) {
		for (auto &fontExt: fontExts) {
			Rml::String fullPath = fontPath + fontExt;
			if (access(fullPath.c_str(), F_OK) != -1) {
				Rml::LoadFontFace(fullPath, true);
			}
		}
	}
}

void Shell::LoadFonts()
{
	const Rml::String directory = "assets/";

	struct FontFace {
		const char* filename;
		bool fallback_face;
	};
	FontFace font_faces[] = {
		{"LatoLatin-Regular.ttf", false},
		{"LatoLatin-Italic.ttf", false},
		{"LatoLatin-Bold.ttf", false},
		{"LatoLatin-BoldItalic.ttf", false},
		{"NotoEmoji-Regular.ttf", true},
	};

	for (const FontFace& face : font_faces)
		Rml::LoadFontFace(directory + face.filename, face.fallback_face);
#if defined(__APPLE__)
    Rml::Vector<Rml::String> simplifiedChineseFonts = {
        "/System/Library/Fonts/PingFang",
    };
#elif defined(_WIN32)
	Rml::Vector<Rml::String> simplifiedChineseFonts = {
		"C:\\Windows\\Fonts\\msyh",
	};
#elif defined(ANDROID)
	Rml::Vector<Rml::String> simplifiedChineseFonts = {
		"/system/fonts/NotoSerifCJK-Regular",
        "/system/fonts/NotoSansCJK-Regular",
        "/system/fonts/NotoSansSC-Regular",
        "/system/fonts/DroidSansFallback",
        "/system/fonts/DroidSansChinese",
    };
#endif
	loadFontsExist(simplifiedChineseFonts);
}

bool Shell::ProcessKeyDownShortcuts(Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority)
{
	if (!context)
		return true;

	// Result should return true to allow the event to propagate to the next handler.
	bool result = false;

	// This function is intended to be called twice by the backend, before and after submitting the key event to the context. This way we can
	// intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority if the context did not
	// intercept it.
	if (priority)
	{
		// Priority shortcuts are handled before submitting the key to the context.

		// Toggle debugger and set dp-ratio using Ctrl +/-/0 keys.
		if (key == Rml::Input::KI_F8)
		{
			Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
		}
		else if (key == Rml::Input::KI_0 && key_modifier & Rml::Input::KM_CTRL)
		{
			context->SetDensityIndependentPixelRatio(1.0f, 1);
		}
		else if (key == Rml::Input::KI_1 && key_modifier & Rml::Input::KM_CTRL)
		{
			const float dp_ratio = context->GetDensityIndependentPixelRatio();
			const float prev_dp_ratio = context->GetDensityIndependentPixelRatio(1);
			context->SetDensityIndependentPixelRatio(1.0f / dp_ratio * prev_dp_ratio, 1);
		}
		else if ((key == Rml::Input::KI_OEM_MINUS || key == Rml::Input::KI_SUBTRACT) && key_modifier & Rml::Input::KM_CTRL)
		{
			const float new_dp_ratio = Rml::Math::Max(context->GetDensityIndependentPixelRatio(1) / 1.2f, 0.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio , 1);
		}
		else if ((key == Rml::Input::KI_OEM_PLUS || key == Rml::Input::KI_ADD) && key_modifier & Rml::Input::KM_CTRL)
		{
			const float new_dp_ratio = Rml::Math::Min(context->GetDensityIndependentPixelRatio(1) * 1.2f, 2.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio, 1);
		}
		else
		{
			// Propagate the key down event to the context.
			result = true;
		}
	}
	else
	{
		// We arrive here when no priority keys are detected and the key was not consumed by the context. Check for shortcuts of lower priority.
		if (key == Rml::Input::KI_R && key_modifier & Rml::Input::KM_CTRL)
		{
			for (int i = 0; i < context->GetNumDocuments(); i++)
			{
				Rml::ElementDocument* document = context->GetDocument(i);
				const Rml::String& src = document->GetSourceURL();
				if (src.size() > 4 && src.substr(src.size() - 4) == ".rml")
				{
					document->ReloadStyleSheet();
				}
			}
		}
		else
		{
			result = true;
		}
	}

	return result;
}
