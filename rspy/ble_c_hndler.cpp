/*
 *
 *  GattLib - GATT Library
 *
 *  Copyright (C) 2016-2017  Olivier Martin <olivier@labapart.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
//Significant modifications added by Cephas Storm, 2021
#include <assert.h>
#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <string>

#include "gattlib.h"

uuid_t g_temp_uuid = uuid_t();
uuid_t g_accel_uuid = uuid_t();
uuid_t g_gyro_uuid = uuid_t();
std::string g_temp_uuid_string = "00002a6e-0000-1000-8000-00805f9b34fb";
std::string g_accel_uuid_string = "00000001-0000-1000-8000-00805f9b34fb";
std::string g_gyro_uuid_string = "00000011-0000-1000-8000-00805f9b34fb";
std::ofstream of;
static GMainLoop* m_main_loop;

void notification_handler(const uuid_t* uuid, const uint8_t* data,
                          size_t data_length, void* user_data) {
  // std::cout << "Length is: " << data_length << std::endl;
  if (data_length == 4) {
    std::cout << "Value: " << *(float*)data << std::endl;
    of << "temp: " << *(float*)data << std::endl;
  } else if (data_length == 4 * sizeof(float)) {
    // std::cout << "This is the gyroscope" << std::endl;
    float* f = ((float*)data);
    of << "gyro: <" << f[0] << "," << f[1] << "," << f[2] << ">" << std::endl;
  } else if (data_length == 3 * sizeof(float)) {
    // std::cout << "This is the accell" << std::endl;
    float* f = ((float*)data);
    of << "accel: <" << f[0] << "," << f[1] << "," << f[2] << ">" << std::endl;
    std::cout << "accel: <" << f[0] << "," << f[1] << "," << f[2] << ">"
              << std::endl;
  } else {
    std::cout << "Unknown error occured. data_length is invalid" << std::endl;
  }
}
void notification_2(const uuid_t* uuid, const uint8_t* data, size_t data_length,
                    void* user_data) {
  std::cout << "Handler 2 called!" << std::endl;
}

static void on_user_abort(int arg) { g_main_loop_quit(m_main_loop); }

static void usage(char* argv[]) { printf("%s <device_address>\n", argv[0]); }

int main(int argc, char* argv[]) {
  of.open("data_out_file.txt", std::ios_base::trunc);
  if (!of.is_open()) {
    std::cout << "Error in opening writing file. Aborting!" << std::endl;
    return -1;
  }
  char DEVICE_ID[18] = "D4:2D:FC:0C:F8:58";
  gattlib_string_to_uuid(g_temp_uuid_string.c_str(),
                         g_temp_uuid_string.length(), &g_temp_uuid);
  gattlib_string_to_uuid(g_accel_uuid_string.c_str(),
                         g_accel_uuid_string.length(), &g_accel_uuid);
  gattlib_string_to_uuid(g_gyro_uuid_string.c_str(),
                         g_gyro_uuid_string.length(), &g_gyro_uuid);

  int ret;
  gatt_connection_t* connection;

  // if (argc != 2) {
  //   usage(argv);
  //   return 1;
  // }

  connection = gattlib_connect(NULL, DEVICE_ID,
                               GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);
  if (connection == NULL) {
    fprintf(stderr, "Failed to connect to the bluetooth device.\n");
    return 1;
  }

  gattlib_register_notification(connection, notification_handler, NULL);

  ret = gattlib_notification_start(connection, &g_temp_uuid);
  if (ret) {
    fprintf(stderr, "Fail to start temp notification.\n");
    goto DISCONNECT;
  }
  ret = gattlib_notification_start(connection, &g_accel_uuid);
  if (ret) {
    fprintf(stderr, "Fail to start accel notification.\n");
    goto DISCONNECT;
  }
  ret = gattlib_notification_start(connection, &g_gyro_uuid);
  if (ret) {
    fprintf(stderr, "Fail to start gyro notification.\n");
    goto DISCONNECT;
  }

  // Catch CTRL-C
  signal(SIGINT, on_user_abort);

  m_main_loop = g_main_loop_new(NULL, 0);
  g_main_loop_run(m_main_loop);

  // In case we quit the main loop, clean the connection
  gattlib_notification_stop(connection, &g_temp_uuid);
  gattlib_notification_stop(connection, &g_accel_uuid);
  gattlib_notification_stop(connection, &g_gyro_uuid);

  g_main_loop_unref(m_main_loop);

DISCONNECT:
  gattlib_disconnect(connection);
  puts("Done");
  return ret;
}
