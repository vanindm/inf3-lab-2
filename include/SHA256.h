#pragma once

#include <openssl/sha.h>
#include <PATypes/Sequence.h>

namespace LabFS_Aux {
	size_t sha256(PATypes::Sequence<char> *sequence) {
		char *buffer = new char[sequence->getLength()];
		char *ptr = buffer;
		auto enumerator = sequence->getEnumerator();
		while(enumerator->moveNext()) {
			(*ptr) = enumerator->current();
		}
		size_t result = 0;
		SHA256((unsigned char*) result, sequence->getLength(), (unsigned char *) &result);
		return result;
	}
};