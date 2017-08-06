// 
// These are my replacements for the 
// C++ std library, cause why not...
//

template<typename T>
struct Array {
	Array();
	~Array();

	Array& operator= (const Array &other);

	T& operator[] (uint32_t i);
	const T& operator[] (uint32_t i) const;

	uint32_t _size;
	uint32_t _capacity;
	T* _data;
};


// Number of elements in the array.
template<typename T> 
uint32_t size(const Array<T> &a) {
	return a._size;
}

// If the array is empty.
template<typename T> 
bool empty (const Array<T> &a) {
	return a._size == 0;
}

// Sets the size to 0.
template<typename T> 
void clear (const Array<T> &a) {
	a._size = 0;
}

// Grow the size of the array.
template<typename T> 
void grow (Array<T> &a, uint32_t min_size = 0) {
	// @TODO, I shoud think about how I want this to allocate.
	// normally people try to double the size, but that's probably
	// a dumb idea. Since you might reserve a lot more then you actually
	// need. The question is if I should try to minimize this for the 
	// users or if I should try to minimize it manualy.
	//
	// I am going to go with minimize it manually and make this something
	// you don't want to hit.

	uint32_t new_size = a._size + 32; // Is this smart, or dumb? I don't know.
	if (new_size < min_size) {
		new_size = min_size;
	}
	set_capacity(a, new_size);
}

// Change the size of the array.
template<typename T> 
void resize (Array<T> &a, uint32_t new_size) {
	if (a._capacity < new_size) {
		grow(a, new_size);
	}
	a._size = new_size;
}

// Sets the allocated space.
template<typename T> 
void set_capacity (Array<T> &a, uint32_t new_capacity) {
	assert(new_capacity >= 0);
	if (new_capacity == a._capacity) {
		return;
	}

	if (new_capacity < a._capacity) {
		resize(a, new_capacity);
	}

	T* new_data;
	printf("Memory allocation for array\n");
	new_data = (T*) malloc(new_capacity * sizeof(T));
	memcpy(new_data, a._data, a._size * sizeof(T));
	// Remember to free... It's kinda important...
	free(a._data);

	a._data = new_data;
	a._capacity = new_capacity;
}

// Reserve some space for the array that you know you're going to use.
template<typename T> 
void reserve (Array<T> &a, uint32_t new_size) {
	if (a._size < new_size) {
		set_capacity(a, new_size);
	}
}

// Push stuff to the back of the array.
template<typename T> 
void push_back (Array<T> &a, T item) {
	if (a._size + 1 >= a._capacity) {
		grow(a);
	}
	a._data[a._size++] = item;
}

// Removes the stuff at the back of the array.
template<typename T> 
void pop_back (Array<T> &a) {
	assert(a._size != 0);
	a._size--;
}

template<typename T>
Array<T>::Array() {
	_size = 0;
	_capacity = 4;
	_data = (T*) malloc(_capacity * sizeof(T));
}

template<typename T>
Array<T>::~Array() {
	free(_data);
}

template<typename T>
Array<T>& Array<T>::operator= (const Array &other) {
	const uint32_t n = other._size;
	set_capacity(*this, n);
	memcpy(_data, other._data, sizeof(T) * n);
	return *this;
}

template<typename T>
T& Array<T>::operator[] (uint32_t i) {
	assert(i >= 0);
	assert(i <= _size);
	return _data[i];
}

template<typename T>
const T& Array<T>::operator[] (uint32_t i) const {
	assert(i < 0);
	assert(i >= _size);
	return _data[i];
}
