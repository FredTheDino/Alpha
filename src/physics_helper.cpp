Vec2 project_along(Shape* s, Vec2 v) {
	Vec2 bounds = {FLT_MAX, -FLT_MAX};
	for (auto p : s->points) {
		float d = dot(p, v);
		bounds._[0] = fmin(bounds._[0], d);
		bounds._[1] = fmax(bounds._[1], d);
	}

	return bounds;
}

void gen_normals_from_points(Array<Vec2> points, Array<Vec2>& normals) {
	int size = points.size();
	Vec2 edge;
	normals.resize(size);
	int curr = 0;

	for (int i = 0; i < size; i++) {
		int j = (i + 1) % size;
		edge = points[i] - points[j];

		if (edge.x == 0 && edge.y == 0) continue;

		bool unique = true;
		for (int i = 0; i < curr; i++) {
			if (dot(normals[i], edge) == 0.0f) {
				unique = false;
				break;
			}
		}

		if (!unique) continue;
		
		Vec2 n = {-edge.y, edge.x}; // Rotation in constructor.
		normals[curr++] = normalize(n);
	}

	normals.resize(curr);
} 

bool can_collide(const Body& a, const Body& b) {
	if ((a.mask & b.mask) == 0) return false;
	if (a.mass == 0 && b.mass == 0) return false;
	return true;
}

void fuse_without_duplicates(const Array<Vec2>& a, const Array<Vec2>& b, 
		Array<Vec2>& out) {
	out = a;
	out.reserve(a.size() + b.size());
	
	for (auto data_b : b) {
		bool unique = true;
		for (auto data_a : a) {
			if (data_a == data_b) {
				unique = false;
				break;
			}
		}
		if (unique) {
			out.push_back(data_b);
		}
	}
}
