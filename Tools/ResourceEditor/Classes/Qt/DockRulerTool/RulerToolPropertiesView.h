/*==================================================================================
 Copyright (c) 2008, DAVA, INC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#ifndef __RESOURCEEDITORQT__RULERTOOLPROPERTIESVIEW__
#define __RESOURCEEDITORQT__RULERTOOLPROPERTIESVIEW__

#include <QWidget>
#include <QDockWidget>
#include "DAVAEngine.h"

using namespace DAVA;

class SceneEditor2;

namespace Ui
{
	class RulerToolPropertiesView;
}

class RulerToolPropertiesView: public QWidget
{
	Q_OBJECT

public:
	explicit RulerToolPropertiesView(QWidget* parent = 0);
	~RulerToolPropertiesView();

	void Init();

private slots:
	void SceneActivated(SceneEditor2* scene);
	void SceneDeactivated(SceneEditor2* scene);
	void RulerToolToggled(SceneEditor2* scene);

	void Toggle();
	void SetLineWidth(int width);

	void UpdateLengths(SceneEditor2* scene, double length, double previewLength);

private:
	Ui::RulerToolPropertiesView* ui;
	SceneEditor2* activeScene;
	QAction* toolbarAction;
	QDockWidget* dockWidget;

	int32 defLineWidthMinValue;
	int32 defLineWidthMaxValue;

	void SetWidgetsState(bool enabled);
	void BlockAllSignals(bool block);
	void UpdateFromScene(SceneEditor2* scene);
};

#endif /* defined(__RESOURCEEDITORQT__RULERTOOLPROPERTIESVIEW__) */
