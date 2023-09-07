import { Client, Wallet, xrpToDrops } from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'

// NOT EXPORTED
import {
  IC,
  ICXRP,
  balance,
  fund,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'
import { MASTER_WALLET } from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers/constants'

import { buy, init, sell, walkPrice } from './utils'

// const XRPL_WSS_ENDPOINT = 'wss://hooks-testnet-v3.xrpl-labs.com'
const XRPL_WSS_ENDPOINT = 'ws://localhost:6006'
const seed = 'sEdTkfapJTnAstJAYRSUX7BYLahrPaJ'
const currency = 'JPN'
const high = 3
const highChange = 1
const low = 1
const lowChange = -1
const max = 3
const maxChange = 5
const min = 1
const minChange = -5

const startPrice = (Math.random() * (high - low) + low).toFixed(2)
const startVolume = (Math.random() * (max - min) + min).toFixed(2)

export async function run(): Promise<void> {
  const client = new Client(XRPL_WSS_ENDPOINT)
  await client.connect()
  client.networkID = await client.getNetworkID()

  const icWallet = Wallet.fromSeed(seed)
  if ((await balance(client, icWallet.classicAddress)) < 5000) {
    await fund(
      client,
      MASTER_WALLET,
      new ICXRP(10000),
      ...[icWallet.classicAddress]
    )
  }
  const oneWallet = Wallet.fromSeed('snQo9w7ZjdiYfS3tUUXab73VTuqwT')
  if ((await balance(client, oneWallet.classicAddress)) < 5000) {
    await fund(
      client,
      MASTER_WALLET,
      new ICXRP(10000),
      ...[oneWallet.classicAddress]
    )
  }
  const twoWallet = Wallet.fromSeed('shJgpGURyv2vVEHNCqMq7ujbR7Sku')
  if ((await balance(client, twoWallet.classicAddress)) < 5000) {
    await fund(
      client,
      MASTER_WALLET,
      new ICXRP(10000),
      ...[twoWallet.classicAddress]
    )
  }

  const ic = IC.gw(currency, icWallet.classicAddress)

  let previousPrice = parseFloat(startPrice)
  let previousVolume = parseFloat(startVolume)

  const price = walkPrice(previousPrice, highChange, lowChange)
  const volume = walkPrice(previousVolume, maxChange, minChange)
  console.log(`NEW PRICE: ${price}`)
  console.log(`NEW VOLUME: ${volume}`)

  const randomBuySell =
    parseFloat((Math.random() * (2 - 0) + 0).toFixed(2)) <= 1
  const randomWallet = parseInt(String(Math.random() * (3 - 0) + 0))
  const wallets = [icWallet, oneWallet, twoWallet]
  const wallet = wallets[randomWallet]

  console.log(`ISSUER: ${ic.issuer}`)
  console.log(`CURRENCY: ${ic.currency}`)
  console.log(`RANDOM BUY/SELL: ${randomBuySell}`)

  // Randomly decide to buy or sell
  if (randomBuySell) {
    console.log(`BUYING with wallet: ${wallet.classicAddress}`)
    ic.set(parseFloat(volume))
    const takerPays: IssuedCurrencyAmount = {
      value: String(ic.value),
      currency: ic.currency as string,
      issuer: ic.issuer as string,
    }
    const takerGets: string = xrpToDrops(
      String(parseInt(String(parseFloat(price) * (ic.value as number))))
    )
    await buy(client, wallet, takerGets, takerPays)
  } else {
    console.log(`SELLING with wallet: ${wallet.classicAddress}`)
    ic.set(parseFloat(volume))
    const takerGets: IssuedCurrencyAmount = {
      value: String(ic.value),
      currency: ic.currency as string,
      issuer: ic.issuer as string,
    }
    const takerPays: string = xrpToDrops(
      String(parseInt(String(parseFloat(price) * (ic.value as number))))
    )
    await sell(client, wallet, takerGets, takerPays)
  }

  await client.disconnect()
}

init(seed, currency)
setInterval(run, 15000)
