#pragma once

namespace LabFS_Aux {
	class IReferenceable {
		virtual void SetReference(void* reference) = 0;
		virtual void* GetReference() = 0;
	};
}