// Copyright 2014 Ruimao Zhang / edited by Junchao
#include <vector>

#include "caffe/common_layers.hpp"
#include "caffe/filler.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template<typename Dtype>
void ElementWiseProductLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
	CHECK_EQ(bottom.size(), 1)<< "EP Layer takes a single blob as input.";
	CHECK_EQ(top.size(), 1) << "EP Layer takes a single blob as output.";
	const int num_output = this->layer_param_.elementwise_product_param().num_output(); //does we need this ?
	biasterm_ = this->layer_param_.elementwise_product_param().bias_term();
	// Figure out the dimensions
	M_ = bottom[0]->num();  // number of simples
	K_ = bottom[0]->count() / bottom[0]->num(); // the dimensional of each feature
	N_ = num_output; // in this layer, K_ equals to N_
	CHECK_EQ(K_, N_);
	top[0]->Reshape(bottom[0]->num(), num_output, 1, 1);

	// Check if we need to set up the weights
	if (this->blobs_.size() > 0) {
		LOG(INFO) << "Skipping parameter initialization";
	} else {
		if (biasterm_) {
			this->blobs_.resize(2);
		} else {
			this->blobs_.resize(1);
		}
		// Intialize the weight
		this->blobs_[0].reset(new Blob<Dtype>(1, 1, 1, K_));
		// fill the weights
		shared_ptr<Filler<Dtype> > weight_filler(
				GetFiller<Dtype>(this->layer_param_.elementwise_product_param().weight_filler()));
		weight_filler->Fill(this->blobs_[0].get());
		// If necessary, intiialize and fill the bias term
		if (biasterm_) {
			this->blobs_[1].reset(new Blob<Dtype>(1, 1, 1, N_));
			shared_ptr<Filler<Dtype> > bias_filler(
					GetFiller<Dtype>(this->layer_param_.elementwise_product_param().bias_filler()));
			bias_filler->Fill(this->blobs_[1].get());
		}
	}  // parameter initialization


//	   // Setting up the bias multiplier
//	if (biasterm_) {
//		LOG(INFO) << "111";
//		bias_multiplier_.reset(new SyncedMemory(N_ * sizeof(Dtype)));
//		Dtype* bias_multiplier_data =
//		reinterpret_cast<Dtype*>(bias_multiplier_->mutable_cpu_data());
//		for (int i = 0; i < N_; ++i) {
//			bias_multiplier_data[i] = 1.;
//		}
//	}
};

template <typename Dtype>
void ElementWiseProductLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
      const vector<Blob<Dtype>*>& top) {
}




template<typename Dtype>
void ElementWiseProductLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
	const Dtype* bottom_data = bottom[0]->cpu_data();
	Dtype* top_data = top[0]->mutable_cpu_data();
	const Dtype* weight = this->blobs_[0]->cpu_data();
	const int num = bottom[0]->num();
	const int dim = bottom[0]->count() / bottom[0]->num();
	

	for ( int i = 0; i < num ; i++ )
	{
		const Dtype* data = bottom_data + i* dim; 		
		caffe_mul( dim, data, weight, top_data + i*dim );
	}
}





template<typename Dtype>
void ElementWiseProductLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
		const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
	const Dtype* top_diff = top[0]->cpu_diff();
	const Dtype* bottom_data = bottom[0]->cpu_data();
	const int num = top[0]->num();
	const int dim = top[0]->count() / top[0]->num();
	Dtype* bottom_diff = bottom[0]->mutable_cpu_diff();
	const Dtype* weight = this->blobs_[0]->cpu_data();

	LOG(INFO) <<"zhangruimao here";


	Blob<Dtype> intermediate_result(1, dim, 1, 1);
	memset(intermediate_result.mutable_cpu_data(), 0, sizeof(Dtype) * dim);



	// Gradient with respect to weight
	for ( int i = 0; i < num ; i++ )
	{
		const Dtype* diff_data = top_diff + i* dim; 
		const Dtype* data = bottom_data + i* dim; 		
		//caffe_mul( dim, diff_data, data, this->blobs_[0]->mutable_cpu_diff() );	
		caffe_mul( dim, diff_data, data, intermediate_result.mutable_cpu_data() );
		caffe_axpy( dim, Dtype(1), intermediate_result.cpu_data(), this->blobs_[0]->mutable_cpu_diff() );
	}


	if (propagate_down[0]) {
		// Gradient with respect to bottom data
		for ( int i = 0; i < num ; i++ )
		{
			const Dtype* diff_data = top_diff + i* dim; 
			Dtype* diff_data_b = bottom_diff + i* dim; 		
			caffe_mul( dim, diff_data, weight, diff_data_b );	
		}
	}
}




#ifdef CPU_ONLY
STUB_GPU(ElementWiseProductLayer);
#endif

INSTANTIATE_CLASS(ElementWiseProductLayer);
REGISTER_LAYER_CLASS(ElementWiseProduct);

}  // namespace caffe
