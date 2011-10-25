/*
 * Copyright � 2011 Daniel Taliun, Christian Fuchsberger and Cristian Pattaro. All rights reserved.
 *
 * This file is part of GWAtoolbox.
 *
 * GWAtoolbox is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GWAtoolbox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GWAtoolbox.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../../include/metas/MetaRatio.h"

const unsigned int MetaRatio::SLICES_CNT = 5;
const double MetaRatio::slices[SLICES_CNT] = {
		0.5, 0.75, 0.95, 0.99, 1.0
};

MetaRatio::MetaRatio(MetaNumeric* source_a, MetaNumeric* source_b) throw (MetaException) : Meta(),
	source_a(source_a), source_b(source_b), skewness(NULL), kurtosis(NULL) {

	skewness = (double*)malloc(MetaRatio::SLICES_CNT * sizeof(double));
	if (skewness == NULL) {
		throw MetaException("MetaRatio", "MetaRatio( MetaNumeric*, MetaNumeric* )", __LINE__, 2, MetaRatio::SLICES_CNT * sizeof(double));
	}

	kurtosis = (double*)malloc(MetaRatio::SLICES_CNT * sizeof(double));
	if (kurtosis == NULL) {
		throw MetaException("WMetaRatio", "MetaRatio( MetaNumeric*, MetaNumeric* )", __LINE__, 2, MetaRatio::SLICES_CNT * sizeof(double));
	}
}

MetaRatio::~MetaRatio() {
	data.clear();
	free(skewness);
	free(kurtosis);
	skewness = NULL;
	kurtosis = NULL;
}

MetaNumeric* MetaRatio::get_source_a() {
	return source_a;
}

MetaNumeric* MetaRatio::get_source_b() {
	return source_b;
}

double* MetaRatio::get_skewness() {
	return skewness;
}

double* MetaRatio::get_kurtosis() {
	return kurtosis;
}

void MetaRatio::put(char* value) throw (MetaException) {
	if (!source_a->is_numeric() || !source_b->is_numeric()) {
		return;
	}

	if (source_a->is_na() || source_b->is_na()) {
		return;
	}

	double ratio = source_a->get_value() / source_b->get_value();
	double p_wald = Rf_pchisq(ratio * ratio, 1.0, 0, 0);
	/*double p_wald = ratio;*/

	data.insert(pair<double, double>(p_wald, source_a->get_value()));
}

void MetaRatio::finalize() throw (MetaException) {
	multimap<double, double>::iterator data_it;
	double* data = NULL;
	unsigned int i = 0;
	int slice = 0;

	double mean = 0.0;
	double sd = 0.0;

	data = (double*)malloc(this->data.size() * sizeof(double));
	if (data == NULL) {
		throw MetaException("MetaRatio", "finalize()", __LINE__, 2, this->data.size() * sizeof(double));
	}

	data_it = this->data.end();
	while (data_it != this->data.begin()) {
		data[i] = data_it->second;
		data_it--;
		i++;
	}

	for (i = 0; i < MetaRatio::SLICES_CNT; i++) {
		slice = (int)(this->data.size() * MetaRatio::slices[i]);

		mean = auxiliary::stats_mean(data, slice);
		sd = auxiliary::stats_sd(data, slice, mean);
		skewness[i] = auxiliary::stats_skewness(data, slice, mean, sd);
		kurtosis[i] = auxiliary::stats_kurtosis(data, slice, mean, sd);
	}

	free(data);
	data = NULL;
	this->data.clear();
}

bool MetaRatio::is_na() {
	return false;
}

void MetaRatio::print(ostream& stream) {

}

void MetaRatio::print_html(ostream& stream, char path_separator) {

}

int MetaRatio::get_memory_usage() {
	int memory = 0;

	memory += data.size() * sizeof(double) * 2;
	memory += MetaRatio::SLICES_CNT * sizeof(double) * 2;

	return memory;
}