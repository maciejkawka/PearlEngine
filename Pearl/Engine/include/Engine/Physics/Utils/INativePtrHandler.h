#pragma once 

namespace PrPhysics {

	class INativePtrHandle {
	public:
		virtual ~INativePtrHandle() = default;

		virtual void* GetNativePtr() = 0;
		virtual void  ReleaseNativePtr() = 0;
	};
}