#pragma once
struct KeyData
{
	// The actual data itself
	vec3 value;
	// Time for this Key
	f64 time;
};

class AnimationNode
{
public:

	// The name of this animation node is associated to
	std::string name;

	std::vector<KeyData> positions;
	std::vector<KeyData> rotations;
	std::vector<KeyData> scales;

public:

	AnimationNode();
	AnimationNode(std::string name);
	AnimationNode(std::string name, std::vector<KeyData> position, std::vector<KeyData> rotation, std::vector<KeyData> scale);
	~AnimationNode();

	void setName(const std::string name) { this->name = name; };
	void addPosition(const vec3 position, f64 time) { positions.push_back({ position, time }); };
	void addRotation(const vec3 rotation, f64 time) { rotations.push_back({ rotation, time }); };
	void addScale(const vec3 scale, f64 time) { scales.push_back({ scale, time }); };
};