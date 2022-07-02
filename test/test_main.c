/**
    This file is part of castspeaker
    Copyright (C) 2022-2028  zwcway

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdlib.h>
#include "check.h"

START_TEST(common_log_level_arg)
  {
    ck_assert_int_eq(log_set_level_from_string("xxx"), -1);
    ck_assert_int_eq(log_set_level_from_string(""), -1);
    ck_assert_int_eq(log_set_level_from_string("0"), 0);
    ck_assert_int_eq(log_set_level_from_string("-1"), -1);
    ck_assert_int_eq(log_set_level_from_string("16"), 0);
    ck_assert_int_eq(log_set_level_from_string("trace"), 0);
    ck_assert_int_eq(log_set_level_from_string("debug"), 0);
    ck_assert_int_eq(log_set_level_from_string("info"), 0);
    ck_assert_int_eq(log_set_level_from_string("warn"), 0);
    ck_assert_int_eq(log_set_level_from_string("error"), 0);
    ck_assert_int_eq(log_set_level_from_string("fatal"), 0);
    ck_assert_int_eq(log_set_level_from_string("server:trace"), 0);
    ck_assert_int_eq(log_set_level_from_string("server:0"), 0);
    ck_assert_int_eq(log_set_level_from_string("server:-1"), -1);
    ck_assert_int_eq(log_set_level_from_string("server:debug"), 0);
    ck_assert_int_eq(log_set_level_from_string("server,"), -1);
    ck_assert_int_eq(log_set_level_from_string(","), -1);
    ck_assert_int_eq(log_set_level_from_string(":"), -1);
    ck_assert_int_eq(log_set_level_from_string("server,trace"), -1);
    ck_assert_int_eq(log_set_level_from_string("server:"), -1);
    ck_assert_int_eq(log_set_level_from_string(":trace"), -1);
    ck_assert_int_eq(log_set_level_from_string("server:trace,server"), -1);
    ck_assert_int_eq(log_set_level_from_string("server:trace,server:"), -1);
    ck_assert_int_eq(log_set_level_from_string("server:trace,server:debug"), 0);
    ck_assert_int_eq(log_set_level_from_string("server:trace,mutex:debug"), 0);
    ck_assert_int_eq(log_set_level_from_string("a:0,b:0,c:0,d:0,e:0,f:0"), 0);
  }
END_TEST

START_TEST(common_ip_ip_stoa)
  {
    ck_assert_int_eq(is_multicast_addr("0.0.0.0"), 0);
    ck_assert_int_eq(is_multicast_addr("224.0.0.0"), 0);
    ck_assert_int_eq(is_multicast_addr("239.0.0.1"), 1);
    ck_assert_int_eq(is_multicast_addr("FF01::1"), 0);
    ck_assert_int_eq(is_multicast_addr("FF02::1"), 1);
  }
END_TEST

void setup(void) {
}

void teardown(void) {
}

Suite *test_suite(void) {
  Suite *s;
  TCase *tc_core;
  TCase *tc_limits;

  s = suite_create("Common");

  /* Core test case */
  tc_core = tcase_create("Log");

  tcase_add_checked_fixture(tc_core, setup, teardown);
  tcase_add_test(tc_core, common_log_level_arg);
  tcase_add_test(tc_core, common_ip_ip_stoa);
  suite_add_tcase(s, tc_core);

  /* Limits test case */
  tc_limits = tcase_create("Limits");

//  tcase_add_test(tc_limits, test_money_create_neg);
//  tcase_add_test(tc_limits, test_money_create_zero);
//  suite_add_tcase(s, tc_limits);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = test_suite();
  sr = srunner_create(s);
  is_multicast_addr("FF05::1");
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return number_failed;
}
