/* Copyright (C) 2014 Odyssee Merveille
odyssee.merveille@gmail.com

	This software is a computer program whose purpose is to compute RORPO.
	This software is governed by the CeCILL-B license under French law and
	abiding by the rules of distribution of free software.  You can  use,
	modify and/ or redistribute the software under the terms of the CeCILL-B
	license as circulated by CEA, CNRS and INRIA at the following URL
	"http://www.cecill.info".

	As a counterpart to the access to the source code and  rights to copy,
	modify and redistribute granted by the license, users are provided only
	with a limited warranty  and the software's author,  the holder of the
	economic rights,  and the successive licensors  have only  limited
	liability.

	In this respect, the user's attention is drawn to the risks associated
	with loading,  using,  modifying and/or developing or reproducing the
	software by the user in light of its specific status of free software,
	that may mean  that it is complicated to manipulate,  and  that  also
	therefore means  that it is reserved for developers  and  experienced
	professionals having in-depth computer knowledge. Users are therefore
	encouraged to load and test the software's suitability as regards their
	requirements in conditions enabling the security of their systems and/or
	data to be ensured and,  more generally, to use and operate it in the
	same conditions as regards security.

	The fact that you are presently reading this means that you have had
	knowledge of the CeCILL-B license and that you accept its terms.
*/

#ifndef IMAGE_INCLUDED
#define IMAGE_INCLUDED

#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>


// ###################################################################################################################
// ############################################### 2D IMAGE ##########################################################
// ###################################################################################################################

template<typename T>
class Image2D {

public :

	Image2D(): m_nDimX(0), m_nDimY(0), m_nSize(0) {}

	Image2D( unsigned int dimX, unsigned int dimY, unsigned int value=0 ):
		 m_nDimX(dimX), m_nDimY(dimY), m_nSize(dimX*dimY), m_vImage(dimX*dimY, value){}

	~Image2D(){}

	T& operator ()( int x, int y) {
		return m_vImage[x + y * m_nDimX];
	}
	const T& operator ()( int x, int y) const {
		return m_vImage[x + y * m_nDimX];
	}

	T& operator ()( int i) {
		return m_vImage[i];
	}
	const T& operator ()( int i) const {
		return m_vImage[i];
	}

	const unsigned int dimX() const {
		return m_nDimX;
	}

	const unsigned int dimY() const {
		return m_nDimY;
	}

	const unsigned int size() const {
		return m_nSize;
	}

	bool empty() const {
		return m_vImage.empty();
	}

	std::vector<T>& get_data(){
		return m_vImage;
	}
	const std::vector<T>& get_data() const {
		return m_vImage;
	}

	const T* get_pointer() const {
		return m_vImage.data();
	}

	T* get_pointer(){
		return m_vImage.data();
	}

	// Fill the image with data pointed by p_Pointer
	void add_data_from_pointer(T* p_Pointer){
		m_vImage.assign(p_Pointer,p_Pointer+size());
	}

	void clear_image(){
		m_vImage.clear();
	}

	private :
		unsigned int m_nDimX;
		unsigned int m_nDimY;
		unsigned int m_nSize;
		std::vector<T> m_vImage;
};


// ###################################################################################################################
// ############################################### 3D IMAGE ##########################################################
// ###################################################################################################################

template<typename T>
class Image3D {

public :

	Image3D(): m_nDimX(0), m_nDimY(0), m_nDimZ(0), m_nSize(0),m_spacingX(1.0),m_spacingY(1.0),m_spacingZ(1.0) {}

	Image3D(unsigned int dimX, unsigned int dimY, unsigned int dimZ,float spacingX,float spacingY,float spacingZ, T value=0):
		m_nDimX(dimX), m_nDimY(dimY), m_nDimZ(dimZ), m_nSize(dimX*dimY*dimZ), 
		m_vImage(dimX*dimY*dimZ, value),
		m_spacingX(spacingX),m_spacingY(spacingY),m_spacingZ(spacingZ){}

	Image3D( unsigned int dimX, unsigned int dimY, unsigned int dimZ, T value=0 ):
		m_nDimX(dimX), m_nDimY(dimY), m_nDimZ(dimZ), m_nSize(dimX*dimY*dimZ), m_vImage(dimX*dimY*dimZ, value),
		m_spacingX(1.0),m_spacingY(1.0),m_spacingZ(1.0){}

	Image3D( const Image3D& image ):
	 	m_nDimX(image.m_nDimX), m_nDimY(image.m_nDimY), m_nDimZ(image.m_nDimZ), m_nSize(image.m_nSize), m_vImage(image.m_vImage),
		m_spacingX(image.m_spacingX),m_spacingY(image.m_spacingY),m_spacingZ(image.m_spacingZ){}

	~Image3D(){}

	T& operator ()( int x, int y, int z ) {
		return m_vImage[x + y * m_nDimX + z * m_nDimX * m_nDimY];
	}
	const T& operator ()( int x, int y, int z ) const {
		return m_vImage[x + y * m_nDimX + z * m_nDimX * m_nDimY];
	}

	T& operator ()( int i ) {
		return m_vImage[i];
	}
	const T& operator ()( int i ) const {
		return m_vImage[i];
	}

    template<typename T2>
    void operator +=( T2 scalar ) {
    	for (auto& val: m_vImage)
            val += scalar;
    }

    template< typename T2 >
    void operator -=( T2 scalar) {
        for (auto& val: m_vImage)
            val -= scalar;
    }

	const unsigned int dimX() const {
		return m_nDimX;
	}

	const unsigned int dimY() const {
		return m_nDimY;
	}

	const unsigned int dimZ() const {
		return m_nDimZ;
	}

	const unsigned int size() const {
		return m_nSize;
	}

	const float spacingX() const{
		return m_spacingX;
	}

	const float spacingY() const{
		return m_spacingY;
	}

	const float spacingZ()const{
		return m_spacingZ;
	}

	bool empty() const {
		return m_vImage.empty();
	}

	std::vector<T>& get_data(){
		return m_vImage;
	}
	const std::vector<T>& get_data() const {
		return m_vImage;
	}

	const T* get_pointer() const {
		return m_vImage.data();
	}

	T* get_pointer(){
		return m_vImage.data();
	}

	// Fill the image with data pointed by p_Pointer
	void add_data_from_pointer( const T* p_Pointer ) {
		m_vImage.assign(p_Pointer,p_Pointer+size());
	}

	void clear_image(){
		m_vImage.clear();
		m_nDimX = m_nDimY = m_nDimZ = 0;
		m_nSize = 0;
	}

	// Return a new image "bordered_image" which is the self image with a "border"-pixel border
	Image3D<T> add_border(int border, int value=0) const {
		Image3D<T> bordered_image(m_nDimX + 2 * border, m_nDimY + 2 * border, m_nDimZ + 2 * border,m_spacingX,m_spacingY,m_spacingZ, value);
		for (int z = 0; z < m_nDimZ ; ++z)
			for (int y = 0 ; y < m_nDimY ; ++y)
				for (int x = 0 ; x < m_nDimX ; ++x)
					bordered_image(x + border, y + border, z + border) = this->operator()(x,y,z);
		return bordered_image;
	}

	// Return a new image with a border of "border" pixels fill with a mirror border condition.
	// Image3D<T> mirror_image( int border )
	// {
	//	 Image3D<T> mirror = add_border(border);
	//	 for (int k = 0 ; k < border ; ++k)
	//		 for (int j = 0 ; j < border ; ++j)
	//			 for (int i = 0 ; i < border ; ++i) {
	//				 mirror(i,j,k) = mirror(m_nDimX+2*border-i-1, m_nDimY+2*border-j-1, m_nDimZ-2*border-k-1);
	//				 mirror(m_nDimX+i,m_nDimY+j,m_nDimZ+k) = mirror(i,j,k);
	//			 }
	//	 return mirror;
	// }

	// Return a new image "bordered_image" which is the self image without ist "border"-pixel border
	void remove_border(int border) {
		int ind=0;
		for (int z = border; z<m_nDimZ - border ; ++z)
			for (int y = border; y<m_nDimY - border; ++y)
				for (int x = border; x<m_nDimX - border; ++x)
					m_vImage[ind++]=m_vImage[x + y * m_nDimX + z * m_nDimX * m_nDimY];

		m_nDimX -= 2 * border;
		m_nDimY -= 2 * border;
		m_nDimZ -= 2 * border;
		m_nSize = m_nDimX * m_nDimY * m_nDimZ;
		m_vImage.resize(size());
	}

	// return a new image which is the copy of this
	const Image3D<unsigned char> copy_image_2_uchar() const {

		Image3D<unsigned char> copy(m_nDimX , m_nDimY , m_nDimZ, m_spacingX, m_spacingY, m_spacingZ);
		auto it1 = m_vImage.begin();
		auto it2 = copy.get_data().begin();
		for ( ; it1 != m_vImage.end() ; ++it1, ++it2 )
					*it2 = (unsigned char)(*it1);
		return copy;
	}

	// return a new image which is the copy of this
	Image3D copy_image() const {
		Image3D copy(m_nDimX , m_nDimY , m_nDimZ,m_spacingX,m_spacingY,m_spacingZ);
		copy.add_data_from_pointer(m_vImage.data());
		return copy;
	}

	// Copy I into this
	void copy_image(Image3D<T> &image) {
		if (image.size() != size())
		{
			m_nDimX = image.dimX();
			m_nDimY = image.dimY();
			m_nDimZ = image.dimZ();
			m_nSize = image.size();
			m_vImage.resize(image.size());

			m_spacingX = image.spacingX();
			m_spacingY = image.spacingY();
			m_spacingZ = image.spacingZ();
		}
		m_vImage.assign(image.get_data().begin(),image.get_data().end());
	}


	// Return the maximum value of this
	int min_value() const {
		return *(std::min_element(m_vImage.begin(),m_vImage.end()));
	}


	// Return the maximum value of this
	int max_value() const {
		return *(std::max_element(m_vImage.begin(),m_vImage.end()));
	}

	// Return the minimum and maximum value of this
	std::pair<T,T> min_max_value() const {
        auto minmax = std::minmax_element(m_vImage.begin(),m_vImage.end());
		return { *minmax.first, *minmax.second };
	}

	// Change the dynamique of image, from [window_min, window_max] to [0, 255]. Intensities smaller than window_min are set to 0 and larger than window_max are set to 255
	void window_dynamic( const T& window_min, const T& window_max ){
		for (auto& val: m_vImage)
        {
			if (val <= window_min)
				val = 0;
			else if (val > window_max)
				 val = 255;
			else
				val = (T)(255 * ((val - (float)window_min) / (window_max - window_min)));
		}
	}

    // Change  the dynamique of image this from [min_value, max_value] to [ 0 , max_value]
	void turn_positive(int min_value, int max_value){
        for (auto& val: m_vImage)
    		val = (T)(max_value * ((val - (float) min_value) / (max_value - min_value)));
	}

	private :
		unsigned int m_nDimX;
		unsigned int m_nDimY;
		unsigned int m_nDimZ;
		unsigned int m_nSize;

		float m_spacingX;
		float m_spacingY;
		float m_spacingZ;

		std::vector<T>m_vImage;
};

template<typename T1, typename T2>
void operator +(Image3D<T1> &image, T2 scalar){
	for (auto& val: image.get_data())
        val += scalar;
}

template<typename T1, typename T2 >
void operator -(Image3D<T1> &image, T2 scalar){
    for (auto& val: image.get_data())
        val -= scalar;
}

#endif // IMAGE_INCLUDED
