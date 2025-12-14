#pragma once

#include <openssl/sha.h>
#include <PATypes/Sequence.h>
#include <memory>

namespace LabFS_Aux {
	size_t pseudoSHA256(PATypes::Sequence<char>* sequence) {
		unsigned char *buffer = new unsigned char[sequence->getLength()];
		unsigned char *ptr = buffer;
		auto enumerator = sequence->getEnumerator();
		while(enumerator->moveNext()) {
			(*ptr) = enumerator->current();
		}
		unsigned char res[32];
		SHA256(buffer, sequence->getLength(), (unsigned char*) res);
		delete enumerator;
		delete[] buffer;
		return *((size_t *) res);
	}
};