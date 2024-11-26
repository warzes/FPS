#pragma once

class ImguiHelper
{
public:
	ImguiHelper();
	~ImguiHelper();

	void Draw();

private:
	std::shared_ptr<Texture> mFontTexture = nullptr;
	Scratch mScratch;
};

class StageViewer : public utils::StageViewer
{
private:
	struct Stage
	{
		std::string name;
		RenderTarget* target;
	};

	std::vector<Stage> mStages;

public:
	void stage(const std::string& name, skygfx::Texture* texture) override;
	void show();
};