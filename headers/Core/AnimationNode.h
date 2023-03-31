#pragma once
struct KeyData
{
	// The actual data itself
	vec3 value;
	// Time for this Key
	f64 time;
};

struct QuatData
{
	// The actual data itself
	quat value;
	// Time for this Key
	f64 time;
};

class AnimationNode
{
public:

	// The name of this animation node is associated to
	std::string name;

	std::vector<KeyData> positions;
	// Special Case: rotation has to be stored as a quaternion, otherwise it won't work
	std::vector<QuatData> rotations;
	std::vector<KeyData> scales;

public:

	AnimationNode();
	AnimationNode(std::string name);
	AnimationNode(std::string name, std::vector<KeyData> position, std::vector<QuatData> rotation, std::vector<KeyData> scale);
	~AnimationNode();

	void setName(const std::string name) { this->name = name; };

	void addPosition(const vec3 position, f64 time) { positions.push_back({ position, time }); };
	const KeyData& getPosition(u32 i) const { return positions[i]; };
	u32 getNumPosition() const { return positions.size(); }

	void addRotation(const quat rotation, f64 time) { rotations.push_back({ rotation, time }); };
	const QuatData& getRotation(u32 i) const { return rotations[i]; };
	u32 getNumRotation() const { return rotations.size(); }

	void addScale(const vec3 scale, f64 time) { scales.push_back({ scale, time }); };
	const KeyData& getScale(u32 i) const { return scales[i]; };
	u32 getNumScale() const { return scales.size(); }
};