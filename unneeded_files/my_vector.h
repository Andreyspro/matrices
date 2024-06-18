#ifndef MY_VECTOR_H
#define MY_VECTOR_H
#define NO_my_vector_debug
#include <iostream>

template <typename T>
class my_vector 
{
public:
	my_vector(); // def. constructor
	my_vector(size_t new_size ); // def. constructor
	my_vector(const my_vector<T>& copy); // copy constructor 
	my_vector(my_vector<T>&& left); //move constructor
	~my_vector();
	const my_vector<T>& operator=(const my_vector<T>& copy); // copy assignment
	const my_vector<T>& operator=(my_vector<T>&& left); // move assignment
	T& operator[](size_t index);
	const T operator[](size_t index) const;

	void resize(size_t new_size);
	size_t size();
private:
	size_t data_size = 0;
	T* data = nullptr;
#ifdef _my_vector_debug
	size_t id;
	static size_t counter;
#endif
};

#ifdef _my_vector_debug
template<typename T> 
size_t my_vector<T>::counter = 0;
#endif

template <typename T>
my_vector<T>::my_vector()
	:data_size(0), data(nullptr)
{
#ifdef _my_vector_debug
	id = my_vector<T>::counter++;
	std::cout << "my_vector Def. constructor ID - " << id << "\n";
#endif
}

template <typename T>
my_vector<T>::my_vector(size_t new_size) // constructor with size
	: data_size(new_size)
{
#ifdef _my_vector_debug
	id = my_vector<T>::counter++;
	std::cout << "my_vector constructor ID - " << id << ", with size - " << new_size << "\n";
#endif
	#ifdef EXTRAOUT
	#endif
	data = new T[new_size];
}

template <typename T>
my_vector<T>::my_vector(const my_vector<T>& copy) // copy constructor
	: data_size(copy.data_size)
{
#ifdef _my_vector_debug
	std::cout << "my_vector Copy constructor from ID - " << copy.id << " to ID - " << id << "\n";
#endif
	delete[] data;
	data = new T[data_size];
	std::copy(copy.data, copy.data + copy.data_size, data);
}

template <typename T>
my_vector<T>::my_vector(my_vector<T> &&left) //move constructor
 : data_size(left.data_size)
{

#ifdef _my_vector_debug
	std::cout << "my_vector Move constructor from ID - " << left.id << " to ID - " << id << "\n";
#endif

	delete[] data;
	data = left.data;
	left.data = nullptr;
	left.data_size = 0;
}

template <typename T>
my_vector<T>::~my_vector()
{
#ifdef _my_vector_debug
	std::cout << "my_vector Dtor ID - " << id << "\n";
#endif
	delete[] data;
}

template <typename T>
const my_vector<T>& my_vector<T>::operator=(const my_vector<T> &copy) // copy assignment
{
#ifdef _my_vector_debug
	std::cout << "my_vector Copy assignment from ID - " << copy.id << " to ID - " << id << "\n";
#endif
	delete[] data;
	data = new T[copy.data_size];
	std::copy(copy.data, copy.data + copy.data_size, data);
	data_size = copy.data_size;
	return *this;
}

template <typename T>
const my_vector<T>& my_vector<T>::operator=(my_vector &&left)
{
#ifdef _my_vector_debug
	std::cout << "my_vector Move assignment from ID - " << left.id << " to ID - " << id << "\n";
#endif
	delete[] data;
	data = left.data;
	left.data = nullptr;
	left.data_size = 0;
	return *this;
}

template <typename T>
T& my_vector<T>::operator[](size_t index)
{
#ifdef _my_vector_debug
	std::cout << "my_vector Setter ID - " << id << ", index - " <<  index << "\n";
#endif
	return data[index];
}

template <typename T>
const T my_vector<T>::operator[](size_t index) const
{
#ifdef _my_vector_debug
	std::cout << "my_vector Getter ID - " << id << ", index - " <<  index << "\n";
#endif
	return data[index];
}

template <typename T>
void my_vector<T>::resize(size_t new_size)
{
#ifdef _my_vector_debug
	std::cout << "my_vector resize ID - " << id << ", size - " <<  new_size << "\n";
#endif	
	delete[] data;
	data = new T[new_size];
	data_size = new_size;
}

template <typename T>
size_t my_vector<T>::size()
{
	return data_size;
}

#endif