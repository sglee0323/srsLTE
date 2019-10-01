/*
 * Copyright 2013-2019 Software Radio Systems Limited
 *
 * This file is part of srsLTE.
 *
 * srsLTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsLTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "srslte/common/log.h"
#include "srsue/hdr/stack/rrc/rrc.h"
#include <map>
#include <string>

#ifndef SRSLTE_RRC_PROCEDURES_H
#define SRSLTE_RRC_PROCEDURES_H

namespace srsue {

class rrc::cell_search_proc
{
public:
  struct cell_search_event_t {
    phy_interface_rrc_lte::cell_search_ret_t cs_ret;
    phy_interface_rrc_lte::phy_cell_t        found_cell;
  };
  enum class state_t { phy_cell_search, si_acquire };

  explicit cell_search_proc(rrc* parent_);
  srslte::proc_outcome_t init();
  srslte::proc_outcome_t step();
  srslte::proc_outcome_t trigger_event(const cell_search_event_t& event);

  phy_interface_rrc_lte::cell_search_ret_t get_cs_ret() const { return search_result.cs_ret; }
  static const char*                       name() { return "Cell Search"; }

private:
  srslte::proc_outcome_t handle_cell_found(const phy_interface_rrc_lte::phy_cell_t& new_cell);

  // conts
  rrc*         rrc_ptr;
  srslte::log* log_h;

  // state vars
  cell_search_event_t search_result;
  state_t             state;
};

class rrc::si_acquire_proc
{
public:
  const static int SIB_SEARCH_TIMEOUT_MS = 1000;

  explicit si_acquire_proc(rrc* parent_);
  srslte::proc_outcome_t init(uint32_t sib_index_);
  srslte::proc_outcome_t step();
  static const char*     name() { return "SI Acquire"; }

private:
  static uint32_t sib_start_tti(uint32_t tti, uint32_t period, uint32_t offset, uint32_t sf);

  // conts
  rrc*         rrc_ptr;
  srslte::log* log_h;

  // state
  uint32_t period = 0, sched_index = 0;
  uint32_t start_tti      = 0;
  uint32_t sib_index      = 0;
  uint32_t last_win_start = 0;
};

class rrc::serving_cell_config_proc
{
public:
  explicit serving_cell_config_proc(rrc* parent_);
  srslte::proc_outcome_t init(const std::vector<uint32_t>& required_sibs_);
  srslte::proc_outcome_t step();
  static const char*     name() { return "Serving Cell Configuration"; }

private:
  rrc*         rrc_ptr;
  srslte::log* log_h;

  // proc args
  std::vector<uint32_t> required_sibs;

  // state variables
  enum class search_state_t { next_sib, si_acquire } search_state;
  uint32_t req_idx = 0;
};

class rrc::cell_selection_proc
{
public:
  explicit cell_selection_proc(rrc* parent_);
  srslte::proc_outcome_t init();
  srslte::proc_outcome_t step();
  void                   on_complete(bool is_success);
  cs_result_t            get_cs_result() const { return cs_result; }
  static const char*     name() { return "Cell Selection"; }

private:
  srslte::proc_outcome_t step_cell_selection();
  srslte::proc_outcome_t step_cell_search();
  srslte::proc_outcome_t step_cell_config();

  // consts
  rrc*         rrc_ptr;
  srslte::log* log_h;

  // state variables
  enum class search_state_t { cell_selection, cell_config, cell_search };
  cs_result_t    cs_result;
  search_state_t state;
  uint32_t       neigh_index;
};

class rrc::plmn_search_proc
{
public:
  explicit plmn_search_proc(rrc* parent_);
  srslte::proc_outcome_t init();
  srslte::proc_outcome_t step();
  void                   on_complete(bool is_success);
  static const char*     name() { return "PLMN Search"; }

private:
  // consts
  rrc*         rrc_ptr;
  srslte::log* log_h;

  // state variables
  found_plmn_t found_plmns[MAX_FOUND_PLMNS];
  int          nof_plmns = 0;
};

class rrc::connection_request_proc
{
public:
  struct cell_selection_complete {
    bool        is_success;
    cs_result_t cs_result;
  };

  explicit connection_request_proc(rrc* parent_);
  srslte::proc_outcome_t
                         init(srslte::establishment_cause_t cause_, srslte::unique_byte_buffer_t dedicated_info_nas_);
  srslte::proc_outcome_t step();
  void                   on_complete(bool is_success);
  srslte::proc_outcome_t trigger_event(const cell_selection_complete& e);
  static const char*     name() { return "Connection Request"; }

private:
  // const
  rrc*         rrc_ptr;
  srslte::log* log_h;
  // args
  srslte::establishment_cause_t cause;
  srslte::unique_byte_buffer_t  dedicated_info_nas;

  // state variables
  enum class state_t { cell_selection, config_serving_cell, wait_t300 } state;
  cs_result_t cs_ret;
};

class rrc::process_pcch_proc
{
public:
  struct paging_complete {
    bool outcome;
  };

  explicit process_pcch_proc(rrc* parent_);
  srslte::proc_outcome_t init(const asn1::rrc::paging_s& paging_);
  srslte::proc_outcome_t step();
  srslte::proc_outcome_t trigger_event(paging_complete e);
  static const char*     name() { return "Process PCCH"; }

private:
  // args
  rrc*                rrc_ptr;
  srslte::log*        log_h;
  asn1::rrc::paging_s paging;

  // vars
  uint32_t paging_idx = 0;
  enum class state_t { next_record, nas_paging, serv_cell_cfg } state;
};

class rrc::go_idle_proc
{
public:
  explicit go_idle_proc(rrc* rrc_);
  srslte::proc_outcome_t init();
  srslte::proc_outcome_t step();
  static const char*     name() { return "Go Idle"; }

private:
  rrc*                  rrc_ptr;
  static const uint32_t rlc_flush_timeout = 2000;

  uint32_t rlc_flush_counter;
};

class rrc::cell_reselection_proc
{
public:
  cell_reselection_proc(rrc* rrc_);
  srslte::proc_outcome_t init();
  srslte::proc_outcome_t step();
  static const char*     name() { return "Cell Reselection"; }

private:
  rrc* rrc_ptr;
};

} // namespace srsue

#endif // SRSLTE_RRC_PROCEDURES_H
