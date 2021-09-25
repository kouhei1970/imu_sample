/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "lsm9ds1_reg.h"

static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_magnetic_field[3];
static float acceleration_mg[3];
static float angular_rate_mdps[3];
static float magnetic_field_mgauss[3];
static lsm9ds1_id_t whoamI;
static lsm9ds1_status_t reg;
static uint8_t rst;
static uint8_t tx_buffer_imu[1000];
static uint8_t tx_buffer_mag[1000];

#define PIN_CSAG  1
#define PIN_MISO  4
#define PIN_CSM   5
#define PIN_SCK   6
#define PIN_MOSI  7

sensbus_t Ins_bus={spi0, PIN_CSAG};
sensbus_t Mag_bus={spi0, PIN_CSM};

stmdev_ctx_t Imu_h;
stmdev_ctx_t Mag_h;

void imu_mag_init(void)
{

  /* Initialize platform specific hardware */
  platform_init(  &Ins_bus,/* INS spi bus & cs pin */
                  &Mag_bus,/* MAG spi bus & cs pin */ 
                  &Imu_h,  /* IMU Handle */
                  &Mag_h,  /* MAG Handle */
                  50*1000, /* SPI Frequency */
                  PIN_MISO,/* MISO Pin number */ 
                  PIN_SCK, /* SCK  Pin number */ 
                  PIN_MOSI,/* MOSI Pin number */ 
                  PIN_CSAG,/* CSAG Pin number */ 
                  PIN_CSM  /* CSM  Pin number */ 
  );

  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  sleep_ms(1000);

  /* Check device ID */
  lsm9ds1_dev_id_get(&Mag_h, &Imu_h, &whoamI);

  if (whoamI.imu != LSM9DS1_IMU_ID || whoamI.mag != LSM9DS1_MAG_ID) {
    while (1) {
      /* manage here device not found */
      printf("Device not found !\n");
      sleep_ms(1000);
    }
  }

  /* Restore default configuration */
  lsm9ds1_dev_reset_set(&Mag_h, &Imu_h, PROPERTY_ENABLE);

  do {
    lsm9ds1_dev_reset_get(&Mag_h, &Imu_h, &rst);
  } while (rst);

  /* Enable Block Data Update */
  lsm9ds1_block_data_update_set(&Mag_h, &Imu_h,
                                PROPERTY_ENABLE);
  /* Set full scale */
  lsm9ds1_xl_full_scale_set(&Imu_h, LSM9DS1_4g);
  lsm9ds1_gy_full_scale_set(&Imu_h, LSM9DS1_2000dps);
  lsm9ds1_mag_full_scale_set(&Mag_h, LSM9DS1_16Ga);

  /* Configure filtering chain - See datasheet for filtering chain details */

  /* Accelerometer filtering chain */
  lsm9ds1_xl_filter_aalias_bandwidth_set(&Imu_h, LSM9DS1_AUTO);
  lsm9ds1_xl_filter_lp_bandwidth_set(&Imu_h,
                                     LSM9DS1_LP_ODR_DIV_50);
  lsm9ds1_xl_filter_out_path_set(&Imu_h, LSM9DS1_LP_OUT);

  /* Gyroscope filtering chain */
  lsm9ds1_gy_filter_lp_bandwidth_set(&Imu_h,
                                     LSM9DS1_LP_ULTRA_LIGHT);
  lsm9ds1_gy_filter_hp_bandwidth_set(&Imu_h, LSM9DS1_HP_MEDIUM);
  lsm9ds1_gy_filter_out_path_set(&Imu_h,
                                 LSM9DS1_LPF1_HPF_LPF2_OUT);

  /* Set Output Data Rate / Power mode */
  lsm9ds1_imu_data_rate_set(&Imu_h, LSM9DS1_IMU_476Hz);
  lsm9ds1_mag_data_rate_set(&Mag_h, LSM9DS1_MAG_MP_560Hz);
}

void imu_mag_data_read(void)
{
  /* Read device status register */
  lsm9ds1_dev_status_get(&Mag_h, &Imu_h, &reg);

  if ( reg.status_imu.xlda && reg.status_imu.gda ) {
    /* Read imu data */
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
    lsm9ds1_acceleration_raw_get(&Imu_h,
                                 data_raw_acceleration);
    lsm9ds1_angular_rate_raw_get(&Imu_h,
                                 data_raw_angular_rate);
    acceleration_mg[0] = lsm9ds1_from_fs4g_to_mg(
                           data_raw_acceleration[0]);
    acceleration_mg[1] = lsm9ds1_from_fs4g_to_mg(
                           data_raw_acceleration[1]);
    acceleration_mg[2] = lsm9ds1_from_fs4g_to_mg(
                           data_raw_acceleration[2]);
    angular_rate_mdps[0] = lsm9ds1_from_fs2000dps_to_mdps(
                             data_raw_angular_rate[0]);
    angular_rate_mdps[1] = lsm9ds1_from_fs2000dps_to_mdps(
                             data_raw_angular_rate[1]);
    angular_rate_mdps[2] = lsm9ds1_from_fs2000dps_to_mdps(
                             data_raw_angular_rate[2]);
  }

  if ( reg.status_mag.zyxda ) {
    /* Read magnetometer data */
    memset(data_raw_magnetic_field, 0x00, 3 * sizeof(int16_t));
    lsm9ds1_magnetic_raw_get(&Mag_h, data_raw_magnetic_field);
    magnetic_field_mgauss[0] = lsm9ds1_from_fs16gauss_to_mG(
                                 data_raw_magnetic_field[0]);
    magnetic_field_mgauss[1] = lsm9ds1_from_fs16gauss_to_mG(
                                 data_raw_magnetic_field[1]);
    magnetic_field_mgauss[2] = lsm9ds1_from_fs16gauss_to_mG(
                                 data_raw_magnetic_field[2]);
  }
  


}


int main(void)
{

  stdio_init_all();
  imu_mag_init();
  while(1)
  {
    imu_mag_data_read();
    sprintf((char *)tx_buffer_imu,
            "IMU-[mg]:\t%4.2f\t%4.2f\t%4.2f\t[mdps]:\t%4.2f\t%4.2f\t%4.2f\t",
            acceleration_mg[0], acceleration_mg[1], acceleration_mg[2],
            angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);
    sprintf((char *)tx_buffer_mag, "MAG-[mG]:\t%4.2f\t%4.2f\t%4.2f\r\n",
            magnetic_field_mgauss[0], magnetic_field_mgauss[1],
            magnetic_field_mgauss[2]);
    tx_com(tx_buffer_imu, strlen((char const *)tx_buffer_imu));
    tx_com(tx_buffer_mag, strlen((char const *)tx_buffer_mag));
    sleep_ms(10);
  }
  return 0;
}
