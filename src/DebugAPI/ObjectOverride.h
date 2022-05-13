

class ClObjectOverride
{
public:
    std::vector<std::string> Path;
};

class ObjectOverride
{
public:
    enum WidgetPos
    {
        Auto,  // default: depends on object size
        Center,
        Top,
        Bottom,
        Root
    };

    std::string ModelName;
	bool HasModelName = false;

	std::string DisplayName;
    bool HasDisplayName = false;

    std::vector<ClObjectOverride> ClObjects;
	WidgetPos WidgetBasePos = WidgetPos::Auto;

	glm::vec3 WidgetPosOffset;
    bool HasWidgetPosOffset = false;

	glm::vec3 WidgetPosOffsetRel;
    bool HasWidgetPosOffsetRel = false;
};
