import { Wallet, xrpToDrops } from '@transia/xrpl'
import { IssuedCurrencyAmount } from '@transia/xrpl/dist/npm/models/common'

// NOT EXPORTED
import {
  IC,
  XrplIntegrationTestContext,
  serverUrl,
  setupClient,
} from '@transia/hooks-toolkit/dist/npm/src/libs/xrpl-helpers'

import { buy, init, sell, walkPrice } from './utils'

const seed = 'sEdTh61ofhxDV8e5higjZygFVtiibBw'
const currency = 'EUR'
const high = 1.4
const highChange = 0.05
const low = 1.2
const lowChange = -0.05
const max = 30
const maxChange = 2
const min = 5
const minChange = -2

const startPrice = (Math.random() * (high - low) + low).toFixed(2)
const startVolume = (Math.random() * (max - min) + min).toFixed(2)

export async function run(): Promise<void> {
  const testContext = (await setupClient(
    serverUrl
  )) as XrplIntegrationTestContext

  const icWallet = Wallet.fromSeed(seed)
  const oneWallet = Wallet.fromSeed('snQo9w7ZjdiYfS3tUUXab73VTuqwT')
  const twoWallet = Wallet.fromSeed('shJgpGURyv2vVEHNCqMq7ujbR7Sku')

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
    await buy(testContext.client, wallet, takerGets, takerPays)
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
    await sell(testContext.client, wallet, takerGets, takerPays)
  }

  await testContext.client.disconnect()
}

init(seed, currency)
setInterval(run, 15000)
